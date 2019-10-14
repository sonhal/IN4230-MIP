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
#include <netinet/in.h>
#include <sys/time.h>
#include <time.h>
#include <inttypes.h>

#include "../../commons/src/polling.h"
#include "../../commons/src/dbg.h"
#include "../../commons/src/application.h"






static long get_milli() {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (long)ts.tv_sec * 1000L + (ts.tv_nsec / 1000000);
}


/*
Client program. Connects to the mipd daemon trough the provided domain socket.
Send the provided message and waits for answere for 1 second using epoll beforing timing out
*/

int main(int argc, char *argv[]){

    int rc = 0;
    struct ping_message *p_message = calloc(1, sizeof(struct ping_message));
    ApplicationMessage *p_response = calloc(1, sizeof(ApplicationMessage));

    int so = 0;

    if(argc < 2){
        printf("ERROR: ping clients needs arguments\nping_client [-h] <destination_host> <message> <socket_application>\n");
        return 1;
    }

    if(!strncmp("-h", argv[1], 2)){
        printf("ping_client [-h] <destination_host> <message> <socket_application>");
        return 0;
    }
    check(argc == 4, "ping_client [-h] <destination_host> <message> <socket_application>");


    rc = sscanf(argv[1], "%" SCNu8 ,  &p_message->dst_mip_addr);
    check(rc != -1, "[PING CLIENT] Failed to parse mip address arg");
    strcpy(p_message->content, argv[2]);

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
    check(rc != -1, "[PING CLIENT] Failed to connect to domain socket: %s",so_name.sun_path);

    printf("[PING CLIENT]  ping message:\ndst:%d\tcontent:%s\n", p_message->dst_mip_addr, p_message->content);
    /* Write works on sockets as well as files: */
    long before = get_milli();
    rc = write(so, p_message, sizeof(struct ping_message));



    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(so, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    rc = recv(so, p_response, sizeof(struct ping_message), 0);
    long after = get_milli();

    if(rc == -1 && errno == EAGAIN){
        printf("[PING CLIENT] Timeout\t time elapsed: %ld\n",after - before);
    }else {
        check(rc != -1, "Failed to read response from mipd");
        printf("[PING CLIENT] received from mipd: %d\tmessage: %s\n", p_response->mip_src, p_response->message.content);
        printf("[PING CLIENT] time elapsed: %ld\n", after - before);
    }

    free(p_message);
    free(p_response);
    close(so);
    return 0;

    error:
        free(p_response);
        free(p_message);
        if(so) close(so);
        return -1;
}