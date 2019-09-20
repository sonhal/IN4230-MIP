#include <stdio.h>
#include <sys/types.h>
#include <sys/un.h>		/* struct sockaddr_un */
#include <unistd.h>
#include <sys/epoll.h> 
#include <string.h>
#include <sys/socket.h>		/* socket, bind, listen, accept */
#include <stdlib.h> 		/* malloc */


#include "polling.h"
#include "app_connection.h"
#include "link.h"
#include "interface.h"
#include "server.h"
#include "dbg.h"

#define MAX_READ 1600
#define MAX_EVENTS 5
#define MAX_STR_BUF 

void startup();
int setup_epoll(struct epoll_event events_to_handle[], int event_num);

int fetch_mip_addresses(int argc, char *argv[], int offset, uint8_t *mip_addresses[], int address_n){
    check((argc - offset - 1) <= address_n, "To many mip addresses provided - max: %d", address_n);
    check((argc - offset) > 0, "To few mip addresses provided - min: 1, %d where provided", (argc - offset));

    int  i = 0;
    int k = 0;
    for ( i = offset +1; i < argc; i++){
        int new_mip_addr = atoi(argv[i]);
        check(new_mip_addr < 256, "MIP address value provided is to large, max: 255");
        debug("mip index %d, set to %d", k, new_mip_addr);
        mip_addresses[k] = new_mip_addr;
        k++;
    }
    return k;

    error:
        return -1;
}

int handle_cli_inputs(int argc, char *argv[], int *is_debug, uint8_t *mip_addresses[], int address_n){
    char *socket_name;
    int offset = 1;
    char *first_arg = argv[1];
    if(!strncmp(first_arg, "-h", 2)){
        printf("help: mipd [-h] [-d] <socket_application> [MIP addresses ...]\n");
    }
    if (!strncmp(first_arg, "-d", 2)){
        // debug mode
        is_debug = 1;
        offset++;
        socket_name = argv[2];
    }else{
        socket_name = argv[1];
    }
    return fetch_mip_addresses(argc, argv, offset, mip_addresses, address_n);
}

int bind_table_to_raw_sockets(struct interface_table *table){
    int rc = 0;
    int i = 0;
    int raw_socket = -1;
    for(i = 0; i < table->size; i++){
        raw_socket = setup_raw_socket();
        check(raw_socket != -1, "Failed to create raw socket");
        rc = bind(raw_socket, table->interfaces[i].so_name, sizeof(struct sockaddr_ll));
        check(rc != -1, "Failed to bind socket to table interface");
        table->interfaces[i].raw_socket = raw_socket;
    }
    return 1;

    error:
        return -1;
}



int main(int argc, char *argv[]){
    char *socket_name = argv[1];
    int local_socket = 0;
    int raw_socket = 0;
    int rc = 0;
    int epoll_fd = 0;
    struct sockaddr_un so_name;
    int num_mip_addresses_args = 0;
    int is_debug = 0;
    struct interface_table *i_table = create_loaded_interface_table();
    uint8_t mip_addresses[i_table->size];
    
    check(argc > 1, "mipd [-h] [-d] <socket_application> [MIP addresses ...]");
    num_mip_addresses_args = handle_cli_inputs(argc, argv, is_debug, mip_addresses, i_table->size);
    check(num_mip_addresses_args != -1, "Exiting...");
    debug("Num addresses: %d", num_mip_addresses_args);
    startup();

    i_table = apply_mip_addresses(i_table, mip_addresses, num_mip_addresses_args);
    rc = bind_table_to_raw_sockets(i_table);
    check(rc != -1, "Failed to setup raw sockets for interfaces");
    printf("Daemon interface table setup\n");
    print_interface_table(i_table);

    //int rc = handle_poll();
    //check(rc != -1, "epolling exited unexpectedly");


    
    local_socket = setup_domain_socket(&so_name, socket_name, strnlen(socket_name, 256));
    check(local_socket != -1, "Failed to create local socket");
    raw_socket = setup_raw_socket();
    check(raw_socket != -1, "Failed to create raw socket");

    struct epoll_event stdin_event = create_epoll_in_event(0);
    struct epoll_event local_domain_event = create_epoll_in_event(local_socket);
    struct epoll_event events_to_handle[] = {stdin_event, local_domain_event};

    epoll_fd = setup_epoll(events_to_handle, 2);
    rc = add_to_table_to_epoll(epoll_fd, i_table);
    check(rc != -1, "Failed to add interfaces to epoll");

    struct server_self *server = init_server_self(local_socket, i_table);
    // MAIN application loop
    struct epoll_event events[MAX_EVENTS];
    rc = start_server(server, epoll_fd, &events, MAX_EVENTS, MAX_READ, 30000);
    //rc = epoll_loop(epoll_fd, local_socket, raw_socket, events, MAX_EVENTS, MAX_READ, 30000);
    //check(rc != -1, "epoll loop exited unexpectedly");

    // Cleanup
    close_open_sockets_on_table_interface(i_table);
    rc = close(epoll_fd);
    check(rc != -1, "Failed to close epoll file descriptor");
    unlink(so_name.sun_path);
    close(local_socket);
    close(raw_socket);
    return 0;

    error:
        close_open_sockets_on_table_interface(i_table);
        if(epoll_fd) close(epoll_fd);
        unlink(so_name.sun_path);
        close(local_socket);
        close(raw_socket);
        return -1;
}

void startup() {
    pid_t pid = getpid();
    log_info("MIP daemon started - pid: %d", pid);
}





