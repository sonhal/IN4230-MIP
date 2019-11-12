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

#include "../../commons/src/mipd_message.h"
#include "../../commons/src/polling.h"
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
    int so = 0;
    struct sockaddr_un so_name;
    BYTE m_buffer[MAX_MIPMESSAGE_SIZE];
    BYTE r_buffer[MAX_MIPMESSAGE_SIZE];
  
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
        
        rc = recv(so, &m_buffer, MAX_MIPMESSAGE_SIZE, 0);
        check(rc != -1, "Failed to read response from mipd");
        check(rc != 0, "mipd disconnected");
        MIPDMessage *message = MIPDMessage_deserialize(&m_buffer);

        log_info("RECEIVED message\tMIP src: %d\tmessage: %s\n", message->mip_address, message->data);

        
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

    close(so);
    return 0;

    error:
        if(so) close(so);
        return -1;
}