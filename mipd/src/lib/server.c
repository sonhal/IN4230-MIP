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
#include "packaging/mip_packet.h"
#include "interface.h"
#include "server.h"


#define INTERFACE_BUF_SIZE 10;
#define MIP_MESSAGE_BUF 1600;

extern void DumpHex(const void* data, size_t size);


void destroy_server_self(struct server_self *self){
    free(self->cache);
    free(self->i_table);
    free(self);
}

struct server_self *init_server_self(int listening_domain_socket, struct interface_table *table, int debug_enabled){
    struct server_self *self;
    self = calloc(1, sizeof(struct server_self));
    self->listening_domain_socket = listening_domain_socket;
    self->connected_domain_socket = -1;
    self->i_table = table;
    self->cache = create_cache();
    self->debug_enabled = debug_enabled;
    return self;
}

void handle_domain_socket_connection(struct server_self *self, int epoll_fd, struct epoll_event *event){
    int rc = 0;

    server_log(self, "Received message on domain socket");
    int new_socket = 0;
    new_socket = accept(event->data.fd, NULL, NULL);
    server_log(self, "new connection bound to socket: %d", new_socket);
    struct epoll_event conn_event = create_epoll_in_event(new_socket);
    rc = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &conn_event);
    check(rc != -1, "Failed to add file descriptor to epoll");
    self->connected_domain_socket = new_socket;

    return;

    error:
        log_warn("Failed to handle domain socket connection");
        return;
}

void handle_domain_socket_disconnect(struct server_self *self, struct epoll_event *event){
    server_log(self, "Client on domain socket: %d disconnected", event->data.fd);
    close(event->data.fd);
}

// returns MIP header tra which descripes the type of package received
int handle_raw_socket_frame(struct server_self *self, struct epoll_event *event, char *read_buffer, int read_buffer_size){
    int rc = 0;
    struct sockaddr_ll *active_interface_so_name;
    struct mip_packet *received_packet = create_empty_mip_packet();
    struct ether_frame *response_e_frame;
    struct mip_header *response_m_header;
    struct mip_packet *response_m_packet;

    rc = recv_raw_mip_packet(event->data.fd, received_packet);
    //rc = receive_raw_mip_packet(event->data.fd, &e_frame, &received_so_name, &received_header);
    check(rc != -1, "Failed to receive from raw socket");
    char *received_packet_str = mip_packet_to_string(received_packet);
    server_log(self, " RECEIVED PACKET:\n%s", received_packet_str);

    int i_pos = get_interface_pos_for_socket(self->i_table, event->data.fd);
    server_log(self, "position found for socket: %d", i_pos);
    int mip_addr = self->i_table->interfaces[i_pos].mip_address;
    active_interface_so_name = self->i_table->interfaces[i_pos].so_name;

    server_log_received_packet(self, &received_packet->m_header);
    if(received_packet->m_header.tra == 1){
        server_log(self, "received header - src: %d\t dest: %d", received_packet->m_header.src_addr, received_packet->m_header.dst_addr);

        response_e_frame = create_ethernet_frame(received_packet->e_frame.src_addr, &active_interface_so_name);
        response_m_header = create_arp_response_package(mip_addr, &received_packet->m_header);
        response_m_packet = create_mip_packet(response_e_frame, response_m_header, NULL, 0);
        rc = sendto_raw_mip_packet(event->data.fd, active_interface_so_name, response_m_packet);
        check(rc != -1, "Failed to send arp response package");
        append_to_cache(self->cache, event->data.fd, received_packet->m_header.src_addr, active_interface_so_name->sll_addr);
    }else if (received_packet->m_header.tra == 0){
        append_to_cache(self->cache, event->data.fd, received_packet->m_header.src_addr, active_interface_so_name->sll_addr);
    }else if (received_packet->m_header.tra == 3){
        debug("Request is transport type request");
        
        rc = write(self->connected_domain_socket, received_packet->message, mip_header_payload_length_in_bytes(received_packet));
        check(rc != -1, "Failed to write received message to domain socket: %d", self->connected_domain_socket);
    }

    if(received_packet) destroy_mip_packet(received_packet);
    return 0;

    error:
        if(received_packet) destroy_mip_packet(received_packet);
        return -1;
}

// Handle a request to send a message on the domain socket
int handle_domain_socket_request(struct server_self *self, int bytes_read, char *read_buffer){
    debug("%zd bytes read\nDomain socket read: %s", bytes_read, read_buffer);
    int rc = 0;
    struct mip_header *m_header = NULL;
    struct ether_frame *e_frame = NULL;
    struct mip_packet *m_packet = NULL;
    struct ping_message *p_message = NULL;

    // Parse message on domain socket
    p_message = parse_ping_request(read_buffer);
    server_log(self, "ping message:\nsrc:%d\tdst:%d\tcontent:%s", p_message->src_mip_addr, p_message->dst_mip_addr, p_message->content);

    int sock = query_mip_address_src_socket(self->cache, p_message->dst_mip_addr);
    check(sock != -1, "could not lockate mip address in cache");

    int i_pos = get_interface_pos_for_socket(self->i_table, sock);
    check(i_pos != -1, "Could not locate sock address in interface table");

    debug("position found for socket: %d", i_pos);

    // set src mip address
    uint8_t src_mip_addr = self->i_table->interfaces[i_pos].mip_address;
    p_message->src_mip_addr = src_mip_addr;

    struct sockaddr_ll *sock_name = self->i_table->interfaces[i_pos].so_name;
    int cache_pos = query_mip_address_pos(self->cache, p_message->dst_mip_addr);
    check(cache_pos != -1, "Could not locate cache pos");


    // Create headers for the message
    e_frame = create_ethernet_frame(self->cache->entries[cache_pos].dst_interface, sock_name);
    debug("src mip addr: %d", src_mip_addr);
    debug("dest mip addr: %d", p_message->dst_mip_addr);
    m_header = create_transport_package(src_mip_addr, p_message->dst_mip_addr);

    // Create MIP packet
    m_packet = create_mip_packet(e_frame, m_header, p_message, sizeof(p_message));

    // Send the message
    rc = sendto_raw_mip_packet(sock, sock_name, m_packet);
    check(rc != -1, "Failed to send transport packet");

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
int start_server(struct server_self *self, int epoll_fd, struct epoll_event *events, int event_num, int read_buffer_size, int timeout){
    int rc = 0;
    int running = 1;
    int event_count = 0;
    size_t bytes_read = 0;
    char read_buffer[read_buffer_size + 1];

    rc = complete_mip_arp(self->i_table);
    check(rc != -1, "Failed to complete mip arp");

    while(running){
        printf("debug value: %d", self->debug_enabled);
        server_log(self,"Polling...\n");
        event_count = epoll_wait(epoll_fd, events, event_num, timeout);
        log_info("%d ready events", event_count);
        int i = 0;
        for(i = 0; i < event_count; i++){
            memset(read_buffer, '\0', read_buffer_size);
            printf("Reading file descriptor [%d] -- ", events[i].data.fd);
            

            // Event on the listening local domain socket, should only be for new connections
            if(events[i].data.fd == self->listening_domain_socket){
                handle_domain_socket_connection(self, epoll_fd, &events[i]);
                continue;
            }

            // Raw socket event
            else if(is_socket_in_table(self->i_table, events[i].data.fd)){
                log_info("RAW SOCKET ACTION");
                handle_raw_socket_frame(self, &events[i], read_buffer, read_buffer_size);
                continue;
            }

            bytes_read = read(events[i].data.fd, read_buffer, read_buffer_size);

            // If the event is not a domain or raw socket and the bytes read is null.
            // The event is a domain socket client. And if the bytes read are 0 the client has disconnected
            if(bytes_read == 0){
                printf("%zd bytes read\n", bytes_read);
                handle_domain_socket_disconnect(self, &events[i]);
                continue;
            } else if(!strncmp(read_buffer, "stop\n", 5)){
                running = 0;
                log_info("Exiting...");
            } else {
                handle_domain_socket_request(self, bytes_read, read_buffer);
            }

        }
        print_cache(self->cache);
    }

    destroy_server_self(self);
    return 1;

    error:
        destroy_server_self(self);
        return -1;
}
