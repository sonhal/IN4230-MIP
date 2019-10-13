#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/epoll.h> 
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <string.h>

#include "../../../commons/src/polling.h"
#include "../../../commons/src/dbg.h"
#include "../../../commons/src/application.h"
#include "app_connection.h"
#include "link.h"
#include "mip_arp.h"
#include "packaging/mip_header.h"
#include "packaging/mip_package.h"
#include "interface.h"
#include "server.h"
#include "router/routing.h"


#define INTERFACE_BUF_SIZE 10
#define MIP_MESSAGE_BUF 1600

extern void DumpHex(const void* data, size_t size);


void MIPDServer_destroy(MIPDServer *server){
    if(server){
        if(server->cache) free(server->cache);
        if(server->i_table) free(server->i_table);
        LocalSocket_destroy(server->route_socket);
        LocalSocket_destroy(server->forward_socket);
        LocalSocket_destroy(server->app_socket);
        free(server);
    }
}

MIPDServer *MIPDServer_create(LocalSocket *app_socket, LocalSocket *route_socket, LocalSocket *forward_socket, struct interface_table *table, int debug_enabled, long cache_update_freq_milli) {
    MIPDServer *server = calloc(1, sizeof(MIPDServer));
    server->app_socket = app_socket;
    server->route_socket = route_socket;
    server->forward_socket = forward_socket;
    server->i_table = table;
    server->cache = create_cache(cache_update_freq_milli);
    server->debug_enabled = debug_enabled;
    return server;
}

// Returns file descriptor for the accepted socket on success, -1 on failure
int handle_domain_socket_connection(MIPDServer *server, int epoll_fd, struct epoll_event *event){
    int rc = 0;

    MIPDServer_log(server, "Received message on domain socket");
    int new_socket = 0;
    new_socket = accept(event->data.fd, NULL, NULL);
    MIPDServer_log(server, "new connection bound to socket: %d", new_socket);
    struct epoll_event conn_event = create_epoll_in_event(new_socket);
    rc = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &conn_event);
    check(rc != -1, "Failed to add file descriptor to epoll");
    return new_socket;

    error:
        log_warn("Failed to handle domain socket connection");
        return -1;
}

void handle_domain_socket_disconnect(MIPDServer *server, struct epoll_event *event){
    MIPDServer_log(server, "Client on domain socket: %d disconnected", event->data.fd);
    close(event->data.fd);
}

// returns MIP header tra which descripes the type of package received
int handle_raw_socket_frame(MIPDServer *server, struct epoll_event *event, char *read_buffer, int read_buffer_size){
    int rc = 0;
    struct sockaddr_ll *active_interface_so_name;
    MIPPackage *received_package = MIPPackage_create_empty();
    struct ether_frame *response_e_frame = NULL;
    struct mip_header *response_m_header = NULL;
    MIPPackage *response_m_packet = NULL;

    rc = recv_raw_mip_package(event->data.fd, received_package);
    //rc = receive_raw_mip_packet(event->data.fd, &e_frame, &received_so_name, &received_header);
    check(rc != -1, "Failed to receive from raw socket");

    int i_pos = get_interface_pos_for_socket(server->i_table, event->data.fd);
    MIPDServer_log(server, "position found for socket: %d", i_pos);
    int mip_addr = server->i_table->interfaces[i_pos].mip_address;
    active_interface_so_name = server->i_table->interfaces[i_pos].so_name;

    MIPDServer_log_received_package(server, &received_package->m_header);



    if (received_package->m_header.tra == 0){
        // MIP arp response
        append_to_cache(server->cache, event->data.fd, received_package->m_header.src_addr, active_interface_so_name->sll_addr);
        print_cache(server->cache);
    } else if(received_package->m_header.tra == 1){
        // MIP arp request
        response_e_frame = create_ethernet_frame(received_package->e_frame.src_addr, &active_interface_so_name);
        response_m_header = create_arp_response_mip_header(mip_addr, &received_package->m_header);
        response_m_packet = MIPPackage_create(response_e_frame, response_m_header, NULL, 0);
        rc = sendto_raw_mip_package(event->data.fd, active_interface_so_name, response_m_packet);
        check(rc != -1, "Failed to send arp response package");
        append_to_cache(server->cache, event->data.fd, received_package->m_header.src_addr, active_interface_so_name->sll_addr);
        // Free the used frames and headers
        free(response_e_frame);
        free(response_m_header);

    } else if (received_package->m_header.tra = 2){
        // MIP route table package
        MIPDServer_log(server, "received route table package");
        recv_route_table_broadcast(server, received_package);


    }else if (received_package->m_header.tra == 3){
        debug("Request is transport type request");
        // LOGG received packet to console
        char *received_package_str = mip_packet_to_string(received_package);
        MIPDServer_log(server, " RECEIVED PACKET:\n%s", received_package_str);
        free(received_package_str);
        
        rc = write(server->app_socket->connected_socket_fd, received_package->message, mip_header_payload_length_in_bytes(received_package));
        check(rc != -1, "Failed to write received message to domain socket: %d", server->app_socket->connected_socket_fd);
    }

    if(received_package) MIPPackage_destroy(received_package);
    return 0;

    error:
        if(received_package) MIPPackage_destroy(received_package);
        return -1;
}

// Handle a request to send a message on the domain socket, returns 1 on success, 0 on non critical error and -1 on critical error
int handle_domain_socket_request(MIPDServer *server, int bytes_read, char *read_buffer){
    int rc = 0;
    struct mip_header *m_header = NULL;
    struct ether_frame *e_frame = NULL;
    struct mip_packet *m_packet = NULL;
    struct ping_message *p_message = NULL;

    // Parse message on domain socket
    p_message = parse_ping_request(read_buffer);
    MIPDServer_log(server, "ping message:\nsrc:%d\tdst:%d\tcontent:%s", p_message->src_mip_addr, p_message->dst_mip_addr, p_message->content);

    int sock = query_mip_address_src_socket(server->cache, p_message->dst_mip_addr);
    if(sock == -1){
        MIPDServer_log(server, "could not lockate mip address: %d in cache", p_message->dst_mip_addr);
        return 0;
    }
    int i_pos = get_interface_pos_for_socket(server->i_table, sock);
    check(i_pos != -1, "Could not locate sock address in interface table");

    // set src mip address
    uint8_t src_mip_addr = server->i_table->interfaces[i_pos].mip_address;
    p_message->src_mip_addr = src_mip_addr;

    struct sockaddr_ll *sock_name = server->i_table->interfaces[i_pos].so_name;
    int cache_pos = query_mip_address_pos(server->cache, p_message->dst_mip_addr);
    check(cache_pos != -1, "Could not locate cache pos");


    // Create headers for the message
    e_frame = create_ethernet_frame(server->cache->entries[cache_pos].dst_interface, sock_name);
    m_header = create_transport_mip_header(src_mip_addr, p_message->dst_mip_addr);

    // Create MIP packet
    p_message->src_mip_addr = m_header->src_addr; // Sett src address in ping message so it can be PONG'ed
    m_packet = MIPPackage_create(e_frame, m_header, p_message, sizeof(p_message));

    // Send the message
    rc = sendto_raw_mip_package(sock, sock_name, m_packet);
    check(rc != -1, "Failed to send transport packet");
    MIPDServer_log(server, " Domain message sent");

    free(p_message);
    free(e_frame);
    free(m_header);
    return 0;

    error:
        if(p_message)free(p_message);
        if(e_frame)free(e_frame);
        if(m_header)free(m_header);
        return -1;
}


/*
Main server function. Starts the server loop and contains the entry for branching into different event handlers
*/
int MIPDServer_run(MIPDServer *server, int epoll_fd, struct epoll_event *events, int event_num, int read_buffer_size, int timeout) {
    int rc = 0;
    int running = 1;
    int event_count = 0;
    size_t bytes_read = 0;
    char read_buffer[read_buffer_size + 1];

    rc = complete_mip_arp(server->i_table, server->cache);
    check(rc != -1, "Failed to complete mip arp");

    while(running){
        MIPDServer_log(server," Polling...");
        event_count = epoll_wait(epoll_fd, events, event_num, timeout);
        int i = 0;
        for(i = 0; i < event_count; i++){
            memset(read_buffer, '\0', read_buffer_size);

            // Event on the listening local domain socket, should only be for new connections
            if(events[i].data.fd == server->app_socket->listening_socket_fd){
                int accepted_socket = handle_domain_socket_connection(server, epoll_fd, &events[i]);
                check(accepted_socket != -1, "Failed to accept connection on domain socket");
                server->app_socket->connected_socket_fd = accepted_socket;
                continue;
            }

            // New route route daemon connection on route socket
            else if(events[i].data.fd == server->route_socket->listening_socket_fd)
            {
                int accepted_socket = handle_domain_socket_connection(server, epoll_fd, &events[i]);
                check(accepted_socket != -1, "Failed to accept connection on domain socket");
                server->route_socket->connected_socket_fd = accepted_socket;
                continue;
            }

            // New route deamon connection on forward socket
            else if(events[i].data.fd == server->forward_socket->listening_socket_fd)
            {
                int accepted_socket = handle_domain_socket_connection(server, epoll_fd, &events[i]);
                check(accepted_socket != -1, "Failed to accept connection on domain socket");
                server->forward_socket->connected_socket_fd = accepted_socket;
                continue;
            }

            // New request from routerd to broadcast route table
            else if(events[i].data.fd == server->route_socket->connected_socket_fd)
            {
                rc = broadcast_route_table(server, events[i].data.fd);
                check(rc != -1, "Failed to broadcast route table");
                continue;
            }

             // New reponse from routerd on forwarding mip packet
            else if(events[i].data.fd == server->forward_socket->connected_socket_fd)
            {
                MIPDServer_log(server, "Route forward event");
                bytes_read = read(events[i].data.fd, read_buffer, read_buffer_size);
                continue;
            }

            // Raw socket event
            else if(is_socket_in_table(server->i_table, events[i].data.fd)){
                MIPDServer_log(server, "raw socket packet");
                handle_raw_socket_frame(server, &events[i], read_buffer, read_buffer_size);
                continue;
            }

            // Application socket event
            else if(events[i].data.fd == server->app_socket->connected_socket_fd){
                MIPDServer_log(server, "application socket event");
                bytes_read = read(events[i].data.fd, read_buffer, read_buffer_size);

                if(bytes_read == 0){
                    handle_domain_socket_disconnect(server, &events[i]);
                } else {
                    rc = handle_domain_socket_request(server, bytes_read, read_buffer);
                    check(rc != -1, "Failed to handle domain socket event");
                }
                continue;
            }

            // stdin event
            else if(events[i].data.fd == 0){
                bytes_read = read(events[i].data.fd, read_buffer, read_buffer_size);
                
                if(!strncmp(read_buffer, "stop\n", 5)){
                    running = 0;
                    log_info("Exiting...");
                } else {
                    printf("unkown command\n");
                }
                continue;
            }

        }
        update_arp_cache(server->i_table, server->cache);
    }

    MIPDServer_destroy(server);
    return 1;

    error:
        MIPDServer_destroy(server);
        return -1;
}
