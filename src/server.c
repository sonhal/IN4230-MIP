#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/epoll.h> 
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include "dbg.h"
#include "link.h"
#include "mip_arp.h"
#include "mip.h"
#include "interface.h"
#include "polling.h"
#include "server.h"

#define INTERFACE_BUF_SIZE 10;

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
    }

    if(e_frame_response)free(e_frame_response);
    if(mip_header_response)free(mip_header_response);
    if(sock_name)free(sock_name);
    return 1;

    error:
        if(e_frame_response)free(e_frame_response);
        if(mip_header_response)free(mip_header_response);
        if(sock_name)free(sock_name);
        return -1;
}


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
            }
            else {
                printf("%zd bytes read\nDomain socket read:\n", bytes_read);
                printf("%s", read_buffer);
            }

            if(!strncmp(read_buffer, "stop\n", 5)){
                running = 0;
                log_info("Exiting...");
            }
        }
    }

    destroy_server_self(self);
    return 1;

    error:
        destroy_server_self(self);
        return -1;
}
