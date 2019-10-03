#include <stdio.h>
#include <sys/types.h>
#include <sys/un.h>		/* struct sockaddr_un */
#include <unistd.h>
#include <sys/epoll.h> 
#include <string.h>
#include <sys/socket.h>		/* socket, bind, listen, accept */
#include <stdlib.h> 		/* malloc */
#include <stdint.h>

#include "../../../commons/src/polling.h"
#include "../lib/app_connection.h"
#include "../lib/link.h"
#include "../lib/interface.h"
#include "../lib/server.h"
#include "../../../commons/src/dbg.h"
#include "cli.h"

#define MAX_READ 1600
#define MAX_EVENTS 5
#define MAX_STR_BUF 64



void print_startup_info() {
    pid_t pid = getpid();
    log_info("MIP daemon started - pid: %d", pid);
}


void clean_up(struct interface_table *i_table, int epoll_fd, struct sockaddr_un *so_name, int local_socket, int raw_socket, struct user_config *u_config, struct epoll_event *events){
    if(i_table) close_open_sockets_on_table_interface(i_table);
    if(epoll_fd) close(epoll_fd);
    if(u_config) destroy_user_config(u_config);
    if(events) free(events);
    unlink(so_name->sun_path);
    close(local_socket);
    close(raw_socket);
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
    int local_socket = 0;
    int raw_socket = 0;
    int rc = 0;
    int epoll_fd = 0;
    struct sockaddr_un so_name;
    struct interface_table *i_table = create_loaded_interface_table();
    struct user_config *u_config = NULL;
    struct epoll_event *events = calloc(MAX_EVENTS, sizeof(struct epoll_event));
    
    check(argc > 1, "mipd [-h] [-d] <socket_application> [MIP addresses ...]");
    if(!strncmp(argv[1], "-h", 2)){
        printf("help: mipd [-h] [-d] <socket_application> [MIP addresses ...]\n");
        return 0;
    }

    // Parse and use user provided configurations
    u_config = handle_user_config(argc, argv, i_table->size);
    check(u_config != NULL, "Exiting...");
    i_table = apply_mip_addresses(i_table, u_config->mip_addresses, u_config->num_mip_addresses);
    local_socket = setup_domain_socket(&so_name, u_config->app_socket, strnlen(u_config->app_socket, 255));
    check(local_socket != -1, "Failed to create local socket");

    print_startup_info();

    rc = bind_table_to_raw_sockets(i_table);
    check(rc != -1, "Failed to setup raw sockets for interfaces");
    printf("Daemon interface table setup\n");
    print_interface_table(i_table);


    raw_socket = setup_raw_socket();
    check(raw_socket != -1, "Failed to create raw socket - daemon must be started with root privileges");

    struct epoll_event stdin_event = create_epoll_in_event(0);
    struct epoll_event local_domain_event = create_epoll_in_event(local_socket);
    struct epoll_event events_to_handle[] = {stdin_event, local_domain_event};

    epoll_fd = setup_epoll(events_to_handle, 2);
    rc = add_to_table_to_epoll(epoll_fd, i_table);
    check(rc != -1, "Failed to add interfaces to epoll");

    struct server_self *server = init_server_self(local_socket, i_table, u_config->is_debug);
    // MAIN application loop
    rc = start_server(server, epoll_fd, events, MAX_EVENTS, MAX_READ, 10000);
    check(rc != -1, "epoll loop exited unexpectedly");

    clean_up(i_table, epoll_fd, &so_name, local_socket, raw_socket, u_config, events);
    return 0;

    error:
        clean_up(i_table, epoll_fd, &so_name, local_socket, raw_socket, u_config, events);
        return -1;
}



