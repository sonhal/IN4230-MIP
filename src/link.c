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
#define SOME_ETH_BROADCAST_ADDR {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
#define PROTOCOL_TYPE 0xff
#define ETH_P_MIP 0x88B5

extern void DumpHex(const void* data, size_t size);

struct mip_header {
    unsigned int t;
    unsigned int r;
    unsigned int a;
    unsigned int ttl;
    unsigned int payload_len;
    uint8_t dst_addr;
    uint8_t src_addr;
} __attribute__((packed));

struct ether_frame {
    uint8_t dst_addr[6];
    uint8_t src_addr[6];
    uint8_t eth_proto[2];
    uint8_t contents[0];
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



int last_inteface(struct sockaddr_ll *so_name){
    int rc = 0;
    struct ifaddrs *ifaces, *ifp;

    rc = getifaddrs(&ifaces);
    check(rc != -1, "Failed to get ip address");

    // Walk the list looking for the ifaces interesting to us
    printf("Interface list:\n");
    for(ifp = ifaces; ifp != NULL; ifp = ifp->ifa_next){
        if(ifp->ifa_addr != NULL && ifp->ifa_addr->sa_family == AF_PACKET){
            // Copy the address info into out temp variable
           memcpy(so_name, (struct sockaddr_ll*)ifp->ifa_addr, sizeof(struct sockaddr_ll));
            //so_name = (struct sockaddr_ll*)ifp->ifa_addr;
            char *addr_str = macaddr_str(so_name);

            printf("%d\t%s\t%s\n", so_name->sll_ifindex,
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
    struct mip_header frame_hdr = {};
    struct iovec msgvec[2];
    uint8_t broad_addr[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

    
        /* Hardcode silly message */
    uint8_t buf[] = {0xde, 0xad, 0xbe, 0xef};

    msg = (struct msghdr *)calloc(1, sizeof(struct msghdr));


    

    /* Fill ethernet header */
    uint8_t broadcast_addr = ETH_BROADCAST_ADDR;
    frame_hdr.dst_addr = broadcast_addr;
    frame_hdr.src_addr = 0xff;

    msgvec[0].iov_base = &frame_hdr;
    msgvec[0].iov_len = sizeof(struct mip_header);
    msgvec[1].iov_base = buf;
    msgvec[1].iov_len = 4;


    //so_name->sll_ifindex = 3;
    // Debug stuff
    debug("Broadcast address: %hhx %hhx %hhx %hhx %hhx %hhx",
     so_name->sll_addr[0],
     so_name->sll_addr[1],
     so_name->sll_addr[2],
     so_name->sll_addr[3],
     so_name->sll_addr[4],
     so_name->sll_addr[5]);
    debug("sll_ifindex: %d", so_name->sll_ifindex);

      /* Fill out message metadata struct */
   
    memcpy(so_name->sll_addr, broad_addr, 6);
    msg->msg_name = so_name;
    msg->msg_namelen = sizeof(struct sockaddr_ll);
    msg->msg_iovlen = 2;
    msg->msg_iov = msgvec;

    rc = sendmsg(sd, msg, 0);
    check(rc != -1, "Failed to send message");

    return 0;

    error:
        return -1;
}


int send_ether_frame_on_raw_socket(int sd, struct sockaddr_ll *so_name, char *message, int message_length){
    int so = sd;
    int rc = 0;
    struct msghdr *msg;
    struct iovec msgvec[2];
    struct ether_frame frame_hdr;


    /* Hardcode silly message */
    uint8_t buf[] = {0xde, 0xad, 0xbe, 0xef};

    /* Fill in Ethernet header */
    uint8_t broadcast_addr[] = SOME_ETH_BROADCAST_ADDR;
    memcpy(frame_hdr.dst_addr, broadcast_addr, 6);
    memcpy(frame_hdr.src_addr, so_name->sll_addr, 6);
    /* Match the ethertype in packet_socket.c: */
    frame_hdr.eth_proto[0] = frame_hdr.eth_proto[1] = 0xFFFF;

    /* Point to frame header */
    msgvec[0].iov_base = &frame_hdr;
    msgvec[0].iov_len = sizeof(struct ether_frame);
    /* Point to frame payload */
    msgvec[1].iov_base = buf;
    msgvec[1].iov_len = 4;

    /* Allocate a zeroed-out message info struct */
    msg = calloc(1, sizeof(struct msghdr));

    /* Fill out message metadata struct */
    memcpy(so_name->sll_addr, broadcast_addr, 6);
    msg->msg_name = so_name;
    msg->msg_namelen = sizeof(struct sockaddr_ll);
    msg->msg_iovlen = 2;
    msg->msg_iov = msgvec;

    printf("Sending %d bytes on if with index: %d\n",
	 rc, so_name->sll_ifindex);

    /* Construct and send message */
    rc = sendmsg(so, msg, 0);
    if (rc == -1) {
    perror("sendmsg");
    free(msg);
    return -1;
    }

     printf("Sent %d bytes on if with index: %d\n",
	 rc, so_name->sll_ifindex);

    
    free(msg);
    return 0;
}

int setup_raw_socket(){
    int so = 0;
    short unsigned int protocol = 0xFFFF;
    short unsigned int mip_protocol = 0x88B5;

    so = socket(AF_PACKET, SOCK_RAW, htons(protocol));
    check(so != -1, "Failed to create raw socket");
    return so;

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