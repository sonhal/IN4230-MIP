#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/epoll.h> 
#include "dbg.h"

int epoll_loop(int epoll_fd, struct epoll_event *events, int event_num, int read_buffer_size, int timeout){

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
