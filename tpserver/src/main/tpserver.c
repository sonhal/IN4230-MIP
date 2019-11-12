#include <sys/time.h>
#include <time.h>
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
#include <errno.h>

#include "../../../commons/src/dbg.h"
#include "../../../commons/src/time_commons.h"
#include "../../../commons/src/client_package.h"


#define TPSERVER_HELP_MESSAGE "tpserver [-h] <miptpd socket> <port>"
#define MAX_DATA_STORE_SIZE 65536

int main(int argc, char *argv[]){

    if(argc < 2){
        printf("invalid input -\n%s\n", TPSERVER_HELP_MESSAGE);
        return 1;
    }

    if(!strncmp("-h", argv[1], 2)){
        printf("%s\n", TPSERVER_HELP_MESSAGE);
        return 0;
    }

    if(!strncmp("-h", argv[1], 2)){
        printf("%s\n", TPSERVER_HELP_MESSAGE);
        return 0;
    }

    log_info("Started TP SERVER...");


    struct sockaddr_un so_name;
    int rc = 0;
    int so = 0;
    uint16_t port = 0;
    BYTE m_buffer[MAX_DATA_STORE_SIZE];

    char *miptp_socket = argv[1];
    sscanf(argv[2], "%d", &port);
    printf("TP SERVER config\tmiptpd socket: %s\tport: %d", miptp_socket, port);


    ClientPackage *package = ClientPackage_create(10, 255, MIPTP_RECEIVER, NULL, 0);
    check(package != NULL, "Failed to create ClientPackage");
  
    /* Create socket */
    so = socket(AF_UNIX, SOCK_STREAM, 0);
    if (so == -1) {
    perror("socket");
    return 1;
    }
    // Set timeout on socket
    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    setsockopt(so, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    /* Zero out name struct */
    memset(&so_name, 0, sizeof(struct sockaddr_un));

    /* Prepare UNIX socket name */
    so_name.sun_family = AF_UNIX;
    strncpy(so_name.sun_path, argv[1], sizeof(so_name.sun_path) - 1);

    rc = connect(so, (const struct sockaddr*)&so_name, sizeof(struct sockaddr_un));
    check(rc != -1, "[TPCLIENT] Failed to connect to domain socket: %s",so_name.sun_path);
    printf("[TPCLIENT] port: %d\tmip destination: %d \n", package->port, package->destination);

    while (1)
    {
        
        rc = recv(so, &m_buffer, MAX_DATA_STORE_SIZE, 0);
        check(rc != -1, "Failed to read response from miptpd");
        check(rc != 0, "miptpd disconnected");
        ClientPackage *package = ClientPackage_deserialize(m_buffer);

        log_info("RECEIVED message\tMIP src: %d\tmessage: %s\n", package->->mip_address, message->data);

        
        char *payload = "PONG";
        MIPDMessage *response = MIPDMessage_create(message->mip_address, strlen(payload), payload);

        log_info("Sending to mip addr: %d", response->mip_address);

        rc =MIPDMessage_serialize(&r_buffer, response);
        check(rc != -1, "Failed to serialize MIPDMessage");

        rc = write(so, &r_buffer, rc);
        check(rc != -1, "Failed to write to mipd");

        // Clean up
        MIPDMessage_destroy(message);
        MIPDMessage_destroy(response);
        memset(m_buffer, 0, MAX_MIPMESSAGE_SIZE);
        memset(r_buffer, 0, MAX_MIPMESSAGE_SIZE);
    }

    ClientPackage_destroy(package);
    if(so)close(so);
    return 0;
    
    error:
        log_err("[MIPTP CLIENT] Failed unexpectedly");
        return 1;
}