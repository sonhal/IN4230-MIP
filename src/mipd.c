#include <stdio.h>
#include <sys/types.h>
#include <sys/un.h>		/* struct sockaddr_un */
#include <unistd.h>
#include <sys/epoll.h> 
#include "dbg.h"
#include "polling.h"
#include "app_connection.h"
#include <string.h>
#include <sys/socket.h>		/* socket, bind, listen, accept */
#include <stdlib.h> 		/* malloc */


#define MAX_READ 1600
#define MAX_EVENTS 5


void startup();
int setup_epoll(struct epoll_event events_to_handle[], int event_num);

int main(int argc, char *argv[]){
    check(argc > 1, "mipd [-h] [-d] <socket_application> [MIP addresses ...]");
    startup();

    //int rc = handle_poll();
    //check(rc != -1, "epolling exited unexpectedly");

    char *socket_name = argv[1];
    int local_socket = 0;
    int raw_socket = 0;
    int rc = 0;
    struct sockaddr_un so_name;
    
    local_socket = setup_domain_socket(&so_name, socket_name, strnlen(socket_name, 256));
    check(local_socket != -1, "Failed to create local socket");
    raw_socket = setup_raw_socket();
    check(raw_socket != -1, "Failed to create raw socket");

    struct epoll_event stdin_event = create_epoll_in_event(0);
    struct epoll_event local_domain_event = create_epoll_in_event(local_socket);
    struct epoll_event raw_socket_event = create_epoll_in_event(raw_socket);
    struct epoll_event events_to_handle[] = {stdin_event, local_domain_event, raw_socket_event};


    int epoll_fd = 0;
    epoll_fd = setup_epoll(&events_to_handle, 3);


    // MAIN application loop
    struct epoll_event events[MAX_EVENTS];
    rc = epoll_loop(epoll_fd, local_socket, raw_socket, events, MAX_EVENTS, MAX_READ, 30000);
    check(rc != -1, "epoll loop exited unexpectedly");

    // Cleanup
    rc = close(epoll_fd);
    check(rc != -1, "Failed to close epoll file descriptor");
    unlink(so_name.sun_path);
    close(socket);
    return 0;

    error:
        if(epoll_fd) close(epoll_fd);
        unlink(so_name.sun_path);
        close(socket);
        return -1;
}

void startup() {
    pid_t pid = getpid();
    log_info("MIP daemon started - pid: %d", pid);
}





