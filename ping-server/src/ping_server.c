#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/socket.h>
#include <linux/if_packet.h>	/* AF_PACKET */
#include <net/ethernet.h>	/* ETH_* */
#include <arpa/inet.h>		/* htons */
#include <ifaddrs.h>		/* getifaddrs */
#include <unistd.h>
#include <sys/un.h>		/* struct sockaddr_un */
#include <sys/epoll.h> 

#include "../../commons/src/polling.h"
#include "../../commons/src/application.h"
#include "../../commons/src/dbg.h"


/*
Ping server program. Connects to the mipd daemon trough the provided domain socket.
*/
int main(int argc, char *argv[]){
    if(argc < 2){
        printf("ping_server [-h] <socket_application>\n");
        return -1;
    }

    if(!strncmp("-h", argv[1], 2)){
        printf("ping_server [-h] <socket_application>\n");
        return 0;
    }

    check(argc == 2, "ping_server [-h] <socket_application>\n");
    log_info("Started Ping server");

    int rc = 0;
    struct ping_message *request = calloc(1, sizeof(struct ping_message));
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


    log_info("Ping server up and polling");
    while (1)
    {
        rc = recv(so, request, sizeof(struct ping_message), 0);
        check(rc != -1, "Failed to read response from mipd");
        log_info("RECEIVED from MIP addr: %d", request->src_mip_addr);
        log_info("Sending to mip addr: %d", request->src_mip_addr);

        struct ping_message *response = calloc(1, sizeof(struct ping_message));
        strncpy(&response->content, "PONG", strlen(&response->content) - 1);
        response->dst_mip_addr = request->src_mip_addr;

        rc = write(so, response, sizeof(struct ping_message));
        check(rc != -1, "Failed to write to mipd");
        memset(request, 0, sizeof(struct ping_message));
    }

    close(so);
    return 0;

    error:
        if(so) close(so);
        return -1;
}