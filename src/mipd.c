#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/epoll.h> 
#include "dbg.h"

#define MAX_READ 5
#define MAX_EVENTS 5

void startup() {

    pid_t pid = getpid();
    log_info("MIP daemon started - pid: %d", pid);
}


int poll_loop(int epoll_fd, struct epoll_event *events, int read_buffer_size, int timeout){

    int running = 1;
    int event_count = 0;
    size_t bytes_read = 0;
    char read_buffer[read_buffer_size + 1];
    while(running){
        printf("Polling for input...\n");
        event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, timeout);
        log_info("%d ready events", event_count);
        int i = 0;
        for(i = 0; i < event_count; i++){
            printf("Reading file descriptor [%d] -- ", events[i].data.fd);
            bytes_read = read(events[i].data.fd, read_buffer, MAX_READ);
            printf("%zd bytes read\n", bytes_read);
            read_buffer[bytes_read] = '\0';
            printf("Read '%s'\n", read_buffer);

            if(!strncmp(read_buffer, "stop\n", 5)){
                running = 0;
                log_info("Exiting...");
            }
        }
    }
    return 1;
}

int main(int argc, char *argv[]){
    startup();

    struct epoll_event event;
    int rc = 0;
    int epoll_fd = 0;
    epoll_fd = epoll_create1(0);
    check(epoll_fd != -1, "Failed to create epoll file descriptor");


    event.events = EPOLLIN;
    event.data.fd = 0;

    rc = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, 0, &event);
    check(rc != -1, "Failed to add file descriptor to epoll");

    struct epoll_event events[MAX_EVENTS];
    rc = poll_loop(epoll_fd, events, MAX_READ, 30000);
    check(rc != -1, "epoll loop exited unexpectedly");

    rc = close(epoll_fd);
    check(rc != -1, "Failed to close epoll file descriptor");
    return 1;

    error:
        if(epoll_fd) close(epoll_fd);
        return -1;

}


