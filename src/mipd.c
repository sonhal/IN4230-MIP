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


#define MAX_READ 5
#define MAX_EVENTS 5


void startup();
int setup_epoll(struct epoll_event events_to_handle[], int event_num);

int main(int argc, char *argv[]){
    check(argc > 1, "mipd [-h] [-d] <socket_application> [MIP addresses ...]");
    startup();

    //int rc = handle_poll();
    //check(rc != -1, "epolling exited unexpectedly");

    char *socket_name = argv[1];
    int socket = 0;
    int rc = 0;
    struct sockaddr_un so_name;
    
    socket = setup_domain_socket(&so_name, socket_name, strnlen(socket_name, 256));


    struct epoll_event stdin_event = create_epoll_in_event(0);
    struct epoll_event local_domain_event = create_epoll_in_event(socket);
    struct epoll_event events_to_handle[] = {stdin_event, local_domain_event};


    int epoll_fd = 0;
    epoll_fd = setup_epoll(&events_to_handle, 2);


    // MAIN application loop
    struct epoll_event events[MAX_EVENTS];
    rc = epoll_loop(epoll_fd, socket, events, MAX_EVENTS, MAX_READ, 30000);
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





