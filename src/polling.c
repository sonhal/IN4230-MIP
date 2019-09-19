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

#define INTERFACE_BUF_SIZE 10;

extern void DumpHex(const void* data, size_t size);


struct server_data {
    int raw_socket;
    int domain_socket;
    uint8_t mip_address;
    struct interface_record interfaces[32];
    struct mip_arp_cache *cache;
};

int destroy_server_data_self(struct server_data *self){
    free(self->cache);
}

struct event_handler
{
    int fd;
    void (*handler_func)(int);
};


struct epoll_event create_epoll_in_event(int fd){
    struct epoll_event new_event;
    new_event.events = EPOLLIN;
    new_event.data.fd = fd;
    return new_event;
}

struct event_handler create_event_handler(int fd, void *handler_func){
    struct event_handler handler;
    handler.fd = fd;
    handler.handler_func = handler_func;
    return handler;
}

int setup_epoll(struct epoll_event events_to_handle[], int event_num){
    int epoll_fd = 0;
    int rc = 0;

    epoll_fd = epoll_create1(0);
    check(epoll_fd != -1, "Failed to create epoll file descriptor");

    int i = 0;
    for(i = 0; i < event_num; i++){
        rc = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, events_to_handle[i].data.fd, &events_to_handle[i]);
        check(rc != -1, "Failed to add file descriptor: %d to epoll", events_to_handle[i].data.fd);
    }

    return epoll_fd;

    error:
        return -1;

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
int handle_raw_socket_frame(struct server_data self, struct epoll_event *event, char *read_buffer, int read_buffer_size){
    int rc = 0;
    struct mip_header received_header;
    struct ether_frame e_frame;
    rc = receive_raw_mip_packet(event->data.fd, &e_frame, &received_header);
    check(rc != -1, "Failed to receive from raw socket");
    debug("%d bytes read\nRAW SOCKET Frame:\n", rc);
    debug("From RAW socket: %s\n", read_buffer);
    if(received_header.tra == 1){
        struct ether_frame *e_frame_response = create_response_ethernet_frame(&e_frame);
        struct mip_header *mip_header_response = create_arp_response_package(self.mip_address, &received_header);
        //rc = send_raw_mip_packet(self.raw_socket, )
    }

    error:
        return;
}

int epoll_loop(int epoll_fd, int local_domain_socket, int raw_socket, struct epoll_event *events, int event_num, int read_buffer_size, int timeout){
    int rc = 0;
    int running = 1;
    int event_count = 0;
    size_t bytes_read = 0;
    char read_buffer[read_buffer_size + 1];
    struct server_data self;
    self.raw_socket = raw_socket;
    self.domain_socket = local_domain_socket;
    self.mip_address = 128;
    self.cache = create_cache();


    // Get interfaces
    int interface_n = 0;
    struct sockaddr_ll *so_name = calloc(10, sizeof(struct sockaddr_ll));
    interface_n = collect_intefaces(so_name, 10);
    //On interfaces to MIP ARP
    rc = complete_mip_arp(so_name, interface_n, raw_socket, 128);

    while(running){
        printf("Polling for input...\n");
        event_count = epoll_wait(epoll_fd, events, event_num, timeout);
        log_info("%d ready events", event_count);
        int i = 0;
        for(i = 0; i < event_count; i++){
            memset(read_buffer, '\0', read_buffer_size);
            printf("Reading file descriptor [%d] -- ", events[i].data.fd);
            

            // Event on the listening local domain socket, should only be for new connections
            if(events[i].data.fd == local_domain_socket){
                handle_domain_socket_connection(epoll_fd, &events[i]);
                continue;
            }

            // Raw socket event
            else if(events[i].data.fd == raw_socket){
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
                struct sockaddr_ll *so_name = malloc(sizeof(struct sockaddr_ll));
                
                memset(so_name, 0, sizeof(struct sockaddr_ll));
                
                rc = last_inteface(so_name);
                check(rc != -1, "Failed to collect interface for raw socket message");

                rc = send_raw_package(raw_socket, so_name, read_buffer, bytes_read);
                check(rc != -1, "Failed to send domain socket message to raw socket");
            }

            if(!strncmp(read_buffer, "stop\n", 5)){
                running = 0;
                log_info("Exiting...");
            }
        }
    }

    destroy_server_data_self(&self);
    return 0;

    error:
        destroy_server_data_self(&self);
        return -1;
}
