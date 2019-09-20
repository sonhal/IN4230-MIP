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
Client program. Connects to the mipd daemon trough the provided domain socket.
Send the provided message and waits for answere for 1 second using epoll beforing timing out
*/

int main(int argc, char *argv[]){

    if(!strncmp("-h", argv[1], 2)){
        printf("ping_client [-h] <destination_host> <message> <socket_application>");
    }

    check(argc == 4, "ping_client [-h] <destination_host> <message> <socket_application>");

    int rc = 0;
    char buffer[256];
    int so = 0;


    strncpy(&buffer, argv[1], sizeof(char) * 3);
    strcat(&buffer, " ");
    strcat(&buffer, argv[2]);
    printf("message: %s\n", &buffer);

    struct sockaddr_un so_name;
  
    /* Create socket */
    so = socket(AF_UNIX, SOCK_STREAM, 0);
    if (so == -1) {
    perror("socket");
    return 1;
    }

    /* Zero out name struct */
    memset(&so_name, 0, sizeof(struct sockaddr_un));

    /* Prepare UNIX socket name */
    so_name.sun_family = AF_UNIX;
    strncpy(so_name.sun_path, argv[3], sizeof(so_name.sun_path) - 1);

    rc = connect(so, (const struct sockaddr*)&so_name, sizeof(struct sockaddr_un));
    check(rc != -1, "Failed to connect to domain socket");
    /* Write works on sockets as well as files: */
    rc = write(so, buffer, strlen(buffer));

    memset(&buffer, 0, 256);
    struct epoll_event so_event = create_epoll_in_event(so);
    struct epoll_event events[] = {so_event};
    int epoll_fd = setup_epoll(events, 1);

    int event_n = 0;
    struct epoll_event events_buf[5];

    event_n = epoll_wait(epoll_fd, &events_buf, 5, 1000);
    if(event_n == 0){
        printf("Timeout\n");
    }else {
        rc = read(so, buffer, strlen(buffer));
        check(rc != -1, "Failed to read repsonse from mipd");
        printf("received from mipd: %s", buffer);
    }

    close(so);

    error:
        if(so) close(so);
        return -1;
}