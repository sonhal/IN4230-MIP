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
#define CACHE_UPDATE_FREQ_MILLI 10000

#define MIPD_HELP_MESSAGE "mipd [-h] [-d] <socket_application> <route_socket> <forward_socket> [MIP addresses ...]"


void print_startup_info() {
    pid_t pid = getpid();
    log_info("MIP daemon started - pid: %d", pid);
}


void clean_up(struct interface_table *i_table, int epoll_fd, struct sockaddr_un *so_name, int raw_socket, struct user_config *u_config, struct epoll_event *events){
    if(i_table) close_open_sockets_on_table_interface(i_table);
    if(epoll_fd) close(epoll_fd);
    if(u_config) UserConfig_destroy(u_config);
    if(events) free(events);
    unlink(so_name->sun_path);
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

    LocalSocket *app_socket = NULL;
    LocalSocket *route_socket = NULL;
    LocalSocket *forward_socket = NULL;

    int rc = 0;
    int epoll_fd = 0;
    struct sockaddr_un so_name;
    struct interface_table *i_table = create_loaded_interface_table();
    UserConfig *u_config = NULL;
    struct epoll_event *events = calloc(MAX_EVENTS, sizeof(struct epoll_event));

    int raw_socket = setup_raw_socket();
    check(raw_socket != -1, "Failed to create raw socket - daemon must be started with root privileges");
    
    check(argc > 1, MIPD_HELP_MESSAGE);
    if(!strncmp(argv[1], "-h", 2)){
        printf(MIPD_HELP_MESSAGE "\n");
        return 0;
    }
    check(argc > 4, MIPD_HELP_MESSAGE);

    // Parse and use user provided configurations
    u_config = UserConfig_from_cli(argc, argv, i_table->size);
    check(u_config != NULL, "Exiting...");
    i_table = apply_mip_addresses(i_table, u_config->mip_addresses, u_config->num_mip_addresses);
    check_mem(i_table);

    app_socket = LocalSocket_create(u_config->app_socket, SOCK_STREAM);
    check_mem(app_socket);
    route_socket = LocalSocket_create(u_config->route_socket, SOCK_STREAM);
    check_mem(route_socket);
    forward_socket = LocalSocket_create(u_config->forward_socket, SOCK_STREAM);
    check_mem(forward_socket);

    print_startup_info();

    rc = bind_table_to_raw_sockets(i_table);
    check(rc != -1, "Failed to setup raw sockets for interfaces");
    printf("Daemon interface table setup\n");
    print_interface_table(i_table);

    struct epoll_event stdin_event = create_epoll_in_event(0);
    struct epoll_event local_domain_event = create_epoll_in_event(app_socket->listening_socket_fd);
    struct epoll_event local_route_event = create_epoll_in_event(route_socket->listening_socket_fd);
    struct epoll_event local_forward_event = create_epoll_in_event(forward_socket->listening_socket_fd);
    struct epoll_event events_to_handle[] = {stdin_event, local_domain_event, local_route_event, local_forward_event};

    epoll_fd = setup_epoll(events_to_handle, 4);
    rc = add_to_table_to_epoll(epoll_fd, i_table);
    check(rc != -1, "Failed to add interfaces to epoll");

    MIPDServer *server = MIPDServer_create(app_socket, route_socket, forward_socket, i_table, u_config->is_debug, CACHE_UPDATE_FREQ_MILLI);
    // MAIN application loop
    rc = MIPDServer_run(server, epoll_fd, events, MAX_EVENTS, MAX_READ, 10000);
    check(rc != -1, "epoll loop exited unexpectedly");

    clean_up(i_table, epoll_fd, &so_name, raw_socket, u_config, events);
    return 0;

    error:
        clean_up(i_table, epoll_fd, &so_name, raw_socket, u_config, events);
        return -1;
}



