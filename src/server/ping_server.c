#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../dbg.h"
#include <sys/socket.h>
#include <linux/if_packet.h>	/* AF_PACKET */
#include <net/ethernet.h>	/* ETH_* */
#include <arpa/inet.h>		/* htons */
#include <ifaddrs.h>		/* getifaddrs */
#include <unistd.h>
#include <sys/un.h>		/* struct sockaddr_un */
#include <sys/epoll.h> 


#include "polling.h"


/*
Ping server program. Connects to the mipd daemon trough the provided domain socket.
*/

int main(int argc, char *argv[]){
    char *pong_message = "PONG";

    if(!strncmp("-h", argv[1], 2)){
        printf("ping_server [-h] <socket_application>");
    }

    check(argc == 2, "ping_server [-h] <socket_application>");
    printf("Started Ping server");

    int rc = 0;
    char buffer[256];
    int so = 0;
    struct sockaddr_un so_name;
  
    /* Create socket */
    so = socket(AF_UNIX, SOCK_STREAM, 0);
    check(so != -1, "Failed to create domain socket");

    /* Zero out name struct */
    memset(&so_name, 0, sizeof(struct sockaddr_un));

    /* Prepare UNIX socket name */
    so_name.sun_family = AF_UNIX;
    strncpy(so_name.sun_path, argv[1], sizeof(so_name.sun_path) - 1);

    rc = connect(so, (const struct sockaddr*)&so_name, sizeof(struct sockaddr_un));
    check(rc != -1, "Failed to connect to domain socket: %s", argv[1]);
    printf("Connected to mipd domain socket at: %d", so);

    struct epoll_event so_event = create_epoll_in_event(so);
    struct epoll_event events[] = {so_event};
    int epoll_fd = setup_epoll(events, 1);

    int event_n = 0;
    struct epoll_event events_buf[5];

    printf("Ping server up and polling");
    while (1)
    {
        event_n = epoll_wait(epoll_fd, &events_buf, 5, 30000);
        if(event_n == 0){
            printf("Ping server polling...");
         }else {
            rc = read(so, buffer, strlen(buffer));
            check(rc != -1, "Failed to read repsonse from mipd");
            printf("pong server received from mipd: %s", buffer);
            rc = write(so, pong_message, strlen(pong_message));
        }
    }
    

    close(so);
    return 0;

    error:
        if(so) close(so);
        return -1;
}