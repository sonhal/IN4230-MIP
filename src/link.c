#include <stdlib.h>		/* free */
#include <stdio.h> 		/* printf */
#include <string.h>		/* memset */
#include <sys/socket.h>		/* socket */
#include <linux/if_packet.h>	/* AF_PACKET */
#include <net/ethernet.h>	/* ETH_* */
#include <arpa/inet.h>		/* htons */
#include <ifaddrs.h>		/* getifaddrs */
#include "dbg.h"

#define BUF_SIZE 1600
#define ETH_BROADCAST_ADDR 0xff
#define PROTOCOL_TYPE = 0xff

extern void DumpHex(const void* data, size_t size);


struct mip_header {
    unsigned int t : 1;
    unsigned int r : 1;
    unsigned int a : 1;
    unsigned int ttl: 4;
    unsigned int payload_len: 9;
    uint8_t dst_addr;
    uint8_t src_addr;
} __attribute__((packed));


char *macaddr_str(struct sockaddr_ll *sa){
    char *macaddr = calloc(6 * 2 + 6, sizeof(char));
    
    int i = 0;
    for (i = 0; i < 6; i++){
        char *buf = strdup(macaddr);

        sprintf(macaddr, "%s%02hhx%s",
                buf,
                sa->sll_addr[i],
                (i < 5) ? ":" : "");

        free(buf);
    }
    return macaddr;
}


void print_interface_list(){
    int rc;
    //struct sockaddr_ll so_name;
    struct ifaddrs *ifaces, *ifp;

    rc = getifaddrs(&ifaces);
    check(rc != -1, "Failed to get ip address");

    // Walk the list looking for the ifaces interesting to us
    printf("Interface list:\n");
    for(ifp = ifaces; ifp != NULL; ifp = ifp->ifa_next){
        if(ifp->ifa_addr != NULL && ifp->ifa_addr->sa_family == AF_PACKET){
            // Copy the address info into out temp variable
           // memcmp(&so_name, (struct sockaddr_ll*)ifp->ifa_addr, sizeof(struct sockaddr_ll));
            struct sockaddr_ll *so_fake = (struct sockaddr_ll*)ifp->ifa_addr;
            char *addr_str = macaddr_str(so_fake);

            printf("%s\t%s\n",
                    ifp->ifa_name != NULL ? ifp->ifa_name : "null", addr_str);
            
            free(addr_str);
        }
    }
    /*  After the loop the address info of the last interface
        enumerated is stored in so_name
    */
   free(ifaces);
   return;

   error:
        log_warn("Failed to print MAC addresses");
        return;
}

int last_inteface(struct sockaddr_ll *so_name){
    int rc;
    struct ifaddrs *ifaces, *ifp;

    rc = getifaddrs(&ifaces);
    check(rc != -1, "Failed to get ip address");

    // Walk the list looking for the ifaces interesting to us
    printf("Interface list:\n");
    for(ifp = ifaces; ifp != NULL; ifp = ifp->ifa_next){
        if(ifp->ifa_addr != NULL && ifp->ifa_addr->sa_family == AF_PACKET){
            // Copy the address info into out temp variable
           memcpy(&so_name, (struct sockaddr_ll*)ifp->ifa_addr, sizeof(struct sockaddr_ll));
            //so_name = (struct sockaddr_ll*)ifp->ifa_addr;
            char *addr_str = macaddr_str(so_name);

            printf("%s\t%s\n",
                    ifp->ifa_name != NULL ? ifp->ifa_name : "null", addr_str);
            
            free(addr_str);
        }
    }
    /*  After the loop the address info of the last interface
        enumerated is stored in so_name
    */
   free(ifaces);
   return 0;

   error:
        return -1;
}


int  send_raw_packet(int sd, struct sockaddr_ll *so_name, char *message, int message_length){
    int rc = 0;
    struct msghdr *msg;
    struct mip_header frame_hdr;
    struct iovec msgvec[2];

    msg = (struct msghdr *)calloc(1, sizeof(struct msghdr));

    /* Hardcode silly message */
    uint8_t buf[] = {0xde, 0xad, 0xbe, 0xef};
    

    /* Fill ethernet header */
    uint8_t broadcast_addr = ETH_BROADCAST_ADDR;
    frame_hdr.dst_addr = broadcast_addr;
    frame_hdr.src_addr = 0xff;

    msgvec[0].iov_base = &frame_hdr;
    msgvec[0].iov_len = sizeof(struct mip_header);
    msgvec[1].iov_base = message;
    msgvec[1].iov_len = message_length;

      /* Fill out message metadata struct */
    //memcpy(so_name->sll_addr, broadcast_addr, 1);
    msg->msg_name = &so_name;
    msg->msg_namelen = sizeof(struct sockaddr_ll);

    msg->msg_iovlen = 2;
    msg->msg_iov = msgvec;

    rc = sendmsg(sd, msg, 0);
    check(rc != -1, "Failed to send message");

    return 0;

    error:
        return -1;
}


/* void print_raw_socket(int socket){
    int rc;
    char buf[BUF_SIZE];

    rc = recv(socket, buf, BUF_SIZE, 0);
    check(rc != -1, "Failed to read raw socket");
    printf("Received Ethernet frame\n");
    DumpHex(buf, rc);

    error:
        return -1;
} */