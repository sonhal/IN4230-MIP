#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/epoll.h> 
#include "dbg.h"
#include "polling.h"
#include "app_connection.h"
#include <string.h>

#define MAX_READ 5
#define MAX_EVENTS 5


void startup();
int handle_poll();

int main(int argc, char *argv[]){
    check(argc > 1, "mipd [-h] [-d] <socket_application> [MIP addresses ...]");
    startup();

    //int rc = handle_poll();
    //check(rc != -1, "epolling exited unexpectedly");

    char *socket_name = argv[1];
    int socket = 0;
    int rc = 0;
    socket = setup_app_socket();
    rc = app_server(socket, socket_name, strlen(socket_name));
    check(rc != -1, "App server failed unexpectedly");

    close(socket);

    error:
        close(socket);
        return -1;

}

void startup() {
    pid_t pid = getpid();
    log_info("MIP daemon started - pid: %d", pid);
}

int handle_poll(){
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
    rc = epoll_loop(epoll_fd, events, MAX_READ, MAX_EVENTS, 30000);
    check(rc != -1, "epoll loop exited unexpectedly");

    rc = close(epoll_fd);
    check(rc != -1, "Failed to close epoll file descriptor");
    return 1;

    error:
        if(epoll_fd) close(epoll_fd);
        return -1;
}




