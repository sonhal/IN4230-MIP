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

int add_to_table_to_epoll(int fd, struct interface_table *table){
    int rc = 0;
    int i = 0;
    int socket = 0;
    for(i = 0; i < table->size; i++){
        socket = table->interfaces[i].raw_socket;
        struct epoll_event i_event = create_epoll_in_event(socket);
        rc = epoll_ctl(fd, EPOLL_CTL_ADD, socket, &i_event);
        check(rc != -1, "Failed to add file descriptor: %d to epoll", socket);
    }
    return 1;

    error:
        return -1;
}

