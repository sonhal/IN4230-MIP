#include <sys/un.h>		/* struct sockaddr_un */
#include <unistd.h>
#include <sys/epoll.h> 
#include <string.h>
#include <sys/socket.h>		/* socket, bind, listen, accept */


#include "../../../commons/src/dbg.h"
#include "../../../commons/src/domain_socket.h"
#include "../../../commons/src/polling.h"


#include "router_server.h"

RouterServer  *RouterServer_create(RouterdConfig *config){
    check(config  != NULL, "Invalid config argument, is NULL");
    RouterServer *server = calloc(1, sizeof(RouterServer));

    server->debug_active = config->debug_active;
    server->broadcast_freq_milli = config->broadcast_freq;
    server->last_broadcast_milli = 0;

    // Instantiate routing table and add local mip addresses
    server->table = MIPRouteTable_create(List_pop(config->mip_addresses));
    LIST_FOREACH(config->mip_addresses, first, next, cur){
        MIP_ADDRESS *address = cur->value;
        MIPRouteTable_update(server->table, *address, 255, 0);
    }

    //Instansiate sockets to default value
    server->epoll_fd = server->forward_fd = server->routing_fd = -1;

    // Copy domain socket file names
    server->routing_domain_sock = calloc(1, strlen(config->routing_socket));
    memcpy(server->routing_domain_sock, config->routing_socket, strlen(config->routing_socket));

    server->forward_domain_sock = calloc(1, strlen(config->forwarding_socket));
    memcpy(server->forward_domain_sock, config->forwarding_socket, strlen(config->forwarding_socket));

    return server;

    error:
        return NULL;
}

void RouterServer_destroy(RouterServer *server){
    if(server){
        if(server->table) MIPRouteTable_destroy(server->table);
        if(server->epoll_fd) close(server->epoll_fd);
        if(server->routing_fd) close(server->routing_fd);
        if(server->forward_fd) close(server->forward_fd);
        free(server);    
    }
}

int RouterServer_init(RouterServer *server){
    int rc = 0;
    server->routing_fd = connect_to_domain_socket(server->routing_domain_sock);
    check(server->routing_fd != -1, "Failed to connect to routing domain socket");

    server->forward_fd = connect_to_domain_socket(server->forward_domain_sock);
    check(server->forward_fd != -1, "Failed to connect to forward domain socket");

    RouterServer_log(server, "Connected to domain sockets\t routing: %s fd: %d \tforwarding: %s fd: %d",
                server->routing_domain_sock, server->routing_fd,
                server->forward_domain_sock, server->forward_fd);
    
    // Setup epoll
    struct epoll_event routing_event = create_epoll_in_event(server->routing_fd);
    struct epoll_event forward_event = create_epoll_in_event(server->forward_fd);
    struct epoll_event events_to_handle[] = {routing_event, forward_event};

    server->epoll_fd = setup_epoll(&events_to_handle, 2);
    check(server->epoll_fd != -1, "Failed to setup epoll");

    return 1;

    error:
        return -1;
}

int RouterServer_run(RouterServer *server){

}

