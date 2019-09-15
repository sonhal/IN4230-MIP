#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/epoll.h> 
#include "dbg.h"


struct epoll_event create_epoll_in_event(int fd){
    struct epoll_event new_event;
    new_event.events = EPOLLIN;
    new_event.data.fd = fd;
    return new_event;
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


int epoll_loop(int epoll_fd, int local_domain_socket, struct epoll_event *events, int event_num, int read_buffer_size, int timeout){
    int rc = 0;
    int running = 1;
    int event_count = 0;
    size_t bytes_read = 0;
    char read_buffer[read_buffer_size + 1];


    while(running){
        printf("Polling for input...\n");
        event_count = epoll_wait(epoll_fd, events, event_num, timeout);
        log_info("%d ready events", event_count);
        int i = 0;
        for(i = 0; i < event_count; i++){
            printf("Reading file descriptor [%d] -- ", events[i].data.fd);
            bytes_read = read(events[i].data.fd, read_buffer, read_buffer_size);

            if(events[i].data.fd == local_domain_socket){
                debug("DOMAIN SOCKET ACTION!");
                int new_socket = 0;
                new_socket = accept(events[i].data.fd, NULL, NULL);
                debug("new connection bound to socket: %d\n", new_socket);
                struct epoll_event conn_event = create_epoll_in_event(new_socket);

                rc = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &conn_event);
                check(rc != -1, "Failed to add file descriptor to epoll");
                continue;
            }
            if(bytes_read == 0){
                close(events[i].data.fd);
                continue;
            }
            printf("%zd bytes read\n", bytes_read);
            read_buffer[bytes_read] = '\0';
            printf("Read '%s'\n", read_buffer);

            if(!strncmp(read_buffer, "stop\n", 5)){
                running = 0;
                log_info("Exiting...");
            }
        }
    }
    return 0;

    error:
        return -1;
}
