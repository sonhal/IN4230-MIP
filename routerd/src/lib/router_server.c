#include <sys/un.h>		/* struct sockaddr_un */
#include <unistd.h>
#include <sys/epoll.h> 
#include <string.h>
#include <sys/socket.h>		/* socket, bind, listen, accept */


#include "../../../commons/src/dbg.h"
#include "../../../commons/src/domain_socket.h"
#include "../../../commons/src/polling.h"
#include "forwarding_handler.h"
#include "routing_handler.h"


#include "router_server.h"


#define PACKET_BUFFER_SIZE 1000
#define EVENTS_BUFFER_SIZE 5
#define POLLING_TIMEOUT 1000

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
    struct epoll_event stdin_event = create_epoll_in_event(0); // listen to stdin for stop message
    struct epoll_event routing_event = create_epoll_in_event(server->routing_fd);
    struct epoll_event forward_event = create_epoll_in_event(server->forward_fd);
    struct epoll_event events_to_handle[] = {stdin_event, routing_event, forward_event};

    server->epoll_fd = setup_epoll(&events_to_handle, 3);
    check(server->epoll_fd != -1, "Failed to setup epoll");

    RouterServer_log(server, "routerd is initialized in debug mode");

    return 1;

    error:
        return -1;
}

int RouterServer_run(RouterServer *server){
    int rc = 0;
    int running = 1;
    int event_count = 0;
    size_t bytes_read = 0;
    char read_buffer[PACKET_BUFFER_SIZE];
    struct epoll_event events[EVENTS_BUFFER_SIZE];

    // [TODO] complete first route broadcast
    //check(rc != -1, "Failed to complete routing broadcast");

    while(running){
        RouterServer_log(server," Polling...");
        event_count = epoll_wait(server->epoll_fd, &events, EVENTS_BUFFER_SIZE, POLLING_TIMEOUT);

        int i = 0;
        for(i = 0; i < event_count; i++){
            memset(read_buffer, '\0', PACKET_BUFFER_SIZE);

            if(events[i].data.fd == server->routing_fd){
                RouterServer_log(server, "Routing event");
                MIPRouteTable *neighbor_table = handle_route_request(server->table, events[i].data.fd);
                check_mem(neighbor_table);
                MIPRouteTable_update_routing(server->table, neighbor_table);
                RouterServer_log(server, "Routing request handled - table from mip address: %d", neighbor_table->table_address);
                continue;
            }

            // Raw socket event
            if(events[i].data.fd == server->forward_fd){
                RouterServer_log(server, "Forward event");
                MIP_ADDRESS result = handle_forwarding_request(server->table, events[i].data.fd);
                send_forwarding_response(events[i].data.fd, result);
                RouterServer_log(server, "Forward response sent - mip address: %d", result);
                continue;
            }

            bytes_read = read(events[i].data.fd, read_buffer, PACKET_BUFFER_SIZE);

            // If the event is not a domain or raw socket and the bytes read is null.
            // The event is a domain socket client. And if the bytes read are 0 the client has disconnected
            if(bytes_read == 0){
                // [TODO] should routed handle a disconnect
                continue;
            } else if(!strncmp(read_buffer, "stop\n", 5)){
                running = 0;
                printf("Exiting...\n");
            }
        }
        
        if(should_complete_route_broadcast(server)){
            rc = broadcast_route_table(server->table, server->routing_fd);
            check(rc != -1, "Failed to broadcast route table");
            server->last_broadcast_milli = get_now_milli();
        }
    }

    return 1;

    error:
        return -1;
}

