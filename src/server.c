#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/epoll.h> 
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>


#include "dbg.h"
#include "app_connection.h"
#include "link.h"
#include "mip_arp.h"
#include "mip.h"
#include "interface.h"
#include "polling.h"
#include "server.h"

#define INTERFACE_BUF_SIZE 10;
#define MIP_MESSAGE_BUF 1600;

extern void DumpHex(const void* data, size_t size);


void destroy_server_self(struct server_self *self){
    free(self->cache);
    free(self->i_table);
    free(self);
}

struct server_self *init_server_self(int domain_socket, struct interface_table *table){
    struct server_self *self;
    self = calloc(1, sizeof(struct server_self));
    self->domain_socket = domain_socket;
    self->i_table = table;
    self->cache = create_cache();
    return self;
}


void handle_domain_socket_connection(int epoll_fd, struct epoll_event *event){
    int rc = 0;

    debug("DOMAIN SOCKET ACTION!");
    int new_socket = 0;
    new_socket = accept(event->data.fd, NULL, NULL);
    debug("new connection bound to socket: %d\n", new_socket);
    struct epoll_event conn_event = create_epoll_in_event(new_socket);

    rc = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &conn_event);
    check(rc != -1, "Failed to add file descriptor to epoll");

    error:
        return;
}

void handle_domain_socket_disconnect(struct epoll_event *event){
    printf("Client on socket: %d disconnected", event->data.fd);
    close(event->data.fd);
}

// returns MIP header tra which descripes the type of package received
int handle_raw_socket_frame(struct server_self *self, struct epoll_event *event, char *read_buffer, int read_buffer_size){
    int rc = 0;
    struct mip_header received_header = {};
    struct sockaddr_ll received_so_name = {};
    struct ether_frame e_frame = {};
    struct ether_frame *e_frame_response = NULL;
    struct mip_header *mip_header_response = NULL;
    struct socketaddr_ll *sock_name = NULL;

    rc = receive_raw_mip_packet(event->data.fd, &e_frame, &received_so_name, &received_header);
    check(rc != -1, "Failed to receive from raw socket");


    debug("%d bytes read\nRAW SOCKET Frame:\n", rc);
    debug("From RAW socket: %s\n", read_buffer);

    int i_pos = get_interface_pos_for_socket(self->i_table, event->data.fd);
    debug("position found for socket: %d", i_pos);
    int mip_addr = self->i_table->interfaces[i_pos].mip_address;
    sock_name = self->i_table->interfaces[i_pos].so_name;

    if(received_header.tra == 1){
        debug("received header - src: %d\t dest: %d", received_header.src_addr, received_header.dst_addr);

        e_frame_response = create_ethernet_frame(e_frame.src_addr, &received_so_name);
        mip_header_response = create_arp_response_package(mip_addr, &received_header);
        check(rc != -1, "Failed to get interface to send return arp on");
        rc = send_raw_mip_packet(event->data.fd, sock_name, e_frame_response, mip_header_response);
        check(rc != -1, "Failed to send arp response package");
        append_to_cache(self->cache, event->data.fd, received_header.src_addr, received_so_name.sll_addr);
    }else if (received_header.tra == 0){
        append_to_cache(self->cache, event->data.fd, received_header.src_addr, received_so_name.sll_addr);
    }else if (received_header.tra == 3){
        char *ping = "PING!";
        write(self->domain_socket, ping, strlen(ping));
    }

    if(e_frame_response)free(e_frame_response);
    if(mip_header_response)free(mip_header_response);
    return 1;

    error:
        if(e_frame_response)free(e_frame_response);
        if(mip_header_response)free(mip_header_response);
        return -1;
}

// Handle a request to send a message on the domain socket
int handle_domain_socket_request(struct server_self *self, int bytes_read, char *read_buffer){
    debug("%zd bytes read\nDomain socket read: %s", bytes_read, read_buffer);
    int rc = 0;
    uint8_t dest_mip_address;
    char *message = calloc(1, sizeof(char) * 64);

    // Parse message on domain socket
    parse_domain_socket_request(read_buffer, &dest_mip_address, message);

    int sock = query_mip_address_src_socket(self->cache, dest_mip_address);
    check(sock != -1, "could not lockate mip address in cache");

    int i_pos = get_interface_pos_for_socket(self->i_table, sock);
    check(i_pos != -1, "Could not locate sock address in interface table");

    debug("position found for socket: %d", i_pos);

    uint8_t src_mip_addr = self->i_table->interfaces[i_pos].mip_address;
    struct sockaddr_ll *sock_name = self->i_table->interfaces[i_pos].so_name;
    int cache_pos = query_mip_address_pos(self->cache, dest_mip_address);
    check(cache_pos != -1, "Could not locate cache pos");


    // Create headers for the message
    struct ether_frame *e_frame = create_ethernet_frame(self->cache->entries[cache_pos].dst_interface, sock_name);
    debug("src mip addr: %d", src_mip_addr);
    debug("dest mip addr: %d", dest_mip_address);
    struct mip_header *m_header = create_transport_package(src_mip_addr, dest_mip_address);

    // Send the message
    rc = send_raw_mip_packet(sock, sock_name, e_frame, m_header);
    check(rc != -1, "Failed to send transport packet");

    free(message);
    free(e_frame);
    free(m_header);
    return 0;

    error:
        free(message);
        free(e_frame);
        free(m_header);
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
        printf("Polling for input...\n");
        event_count = epoll_wait(epoll_fd, events, event_num, timeout);
        log_info("%d ready events", event_count);
        int i = 0;
        for(i = 0; i < event_count; i++){
            memset(read_buffer, '\0', read_buffer_size);
            printf("Reading file descriptor [%d] -- ", events[i].data.fd);
            

            // Event on the listening local domain socket, should only be for new connections
            if(events[i].data.fd == self->domain_socket){
                handle_domain_socket_connection(epoll_fd, &events[i]);
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
                handle_domain_socket_disconnect(&events[i]);
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
