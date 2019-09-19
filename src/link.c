#include <stdlib.h>		/* free */
#include <stdio.h> 		/* printf */
#include <string.h>		/* memset */
#include <sys/socket.h>		/* socket */
#include <linux/if_packet.h>	/* AF_PACKET */
#include <net/ethernet.h>	/* ETH_* */
#include <arpa/inet.h>		/* htons */
#include <ifaddrs.h>		/* getifaddrs */

#include "interface.h"
#include "dbg.h"
#include "mip.h"
#include "link.h"


#define BUF_SIZE 1600
#define ETH_BROADCAST_ADDR {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
#define PROTOCOL_TYPE 0xff
#define ETH_P_MIP 0x88B5

extern void DumpHex(const void* data, size_t size);


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


int receive_raw_packet(int sd, char *buf, size_t len)
{
    struct sockaddr_ll  so_name;
    struct ether_frame  frame_hdr;
    struct msghdr       msg;
    struct iovec        msgvec[2];
    int 			    rc = 0;

    /* Point to frame header */
    msgvec[0].iov_base = &frame_hdr;
    msgvec[0].iov_len  = sizeof(struct ether_frame);
    /* Point to frame payload */
    msgvec[1].iov_base = buf;
    msgvec[1].iov_len  = len;

    /* Fill out message metadata struct */
    //memcpy(so_name->sll_addr, dst_addr, 6);
    msg.msg_name    = &so_name;
    msg.msg_namelen = sizeof(struct sockaddr_ll);
    msg.msg_iovlen  = 2;
    msg.msg_iov     = msgvec;

    rc = recvmsg(sd, &msg, 0);
    check(rc != -1, "Failed to receive message from raw socket");

    /*
    * Copy the src_addr of the current frame to the global dst_addr
    * We need that address as a dst_addr for the next frames we're going to send from the server
    *  memcpy(dst_addr,frame_hdr.src_addr, 6);
    */
    return rc;

    error:
        return -1;
}

int receive_raw_mip_packet(int sd, struct ether_frame  *frame_hdr, struct mip_header *header){
    struct sockaddr_ll  so_name;
    struct msghdr       msg;
    struct iovec        msgvec[2];
    int 			    rc = 0;
    char                *src_mac;

    /* Point to frame header */
    msgvec[0].iov_base = frame_hdr;
    msgvec[0].iov_len  = sizeof(struct ether_frame);
    /* Point to frame payload */
    msgvec[1].iov_base = header;
    msgvec[1].iov_len  = sizeof(struct mip_header);

    /* Fill out message metadata struct */
    //memcpy(so_name->sll_addr, dst_addr, 6);
    msg.msg_name    = &so_name;
    msg.msg_namelen = sizeof(struct sockaddr_ll);
    msg.msg_iovlen  = 2;
    msg.msg_iov     = msgvec;

    rc = recvmsg(sd, &msg, 0);
    check(rc != -1, "Failed to receive message from raw socket");
    src_mac = macaddr_str(&so_name);

    log_info("Received mip packet with tra %d\tmip address %d\tsrc mac %s", header->tra, header->src_addr, src_mac);

    /*
    * Copy the src_addr of the current frame to the global dst_addr
    * We need that address as a dst_addr for the next frames we're going to send from the server
    *  memcpy(dst_addr,frame_hdr.src_addr, 6);
    */
    free(src_mac);
    return rc;

    error:
        free(src_mac);
        return -1;
}


int send_raw_package(int sd, struct sockaddr_ll *so_name, char *message, int message_length){
    int so = sd;
    int rc = 0;
    struct msghdr *msg;
    struct iovec msgvec[2];
    struct ether_frame frame_hdr;


    /* Hardcode silly message */
    uint8_t buf[] = {0xde, 0xad, 0xbe, 0xef};

    /* Fill in Ethernet header */
    uint8_t broadcast_addr[] = ETH_BROADCAST_ADDR;
    memcpy(frame_hdr.dst_addr, broadcast_addr, 6);
    memcpy(frame_hdr.src_addr, so_name->sll_addr, 6);
    /* Match the ethertype in packet_so9cket.c: */
    frame_hdr.eth_proto[0] = 0x88;
    frame_hdr.eth_proto[1] = 0xB5;


    log_info("SENDING WITH PROTOCOL: %hhx%hhx", frame_hdr.eth_proto[0], frame_hdr.eth_proto[1]);

    /* Point to frame header */
    msgvec[0].iov_base = &frame_hdr;
    msgvec[0].iov_len = sizeof(struct ether_frame);
    /* Point to frame payload */
    msgvec[1].iov_base = message;
    msgvec[1].iov_len = message_length;

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

int send_raw_mip_packet(int sd, struct sockaddr_ll *so_name, struct ether_frame *frame_hdr, struct mip_header *mip_header){
    int so = sd;
    int rc = 0;
    struct msghdr *msg;
    struct iovec msgvec[2];


    /* Hardcode silly message */
    uint8_t buf[] = {0xde, 0xad, 0xbe, 0xef};

    /* Fill in Ethernet header */
    uint8_t broadcast_addr[] = ETH_BROADCAST_ADDR;
    memcpy(frame_hdr->dst_addr, broadcast_addr, 6);
    memcpy(frame_hdr->src_addr, so_name->sll_addr, 6);
    /* Match the ethertype in packet_so9cket.c: */
    frame_hdr->eth_proto[0] = 0x88;
    frame_hdr->eth_proto[1] = 0xB5;


    log_info("SENDING WITH PROTOCOL: %hhx%hhx", frame_hdr->eth_proto[0], frame_hdr->eth_proto[1]);

    /* Point to frame header */
    msgvec[0].iov_base = frame_hdr;
    msgvec[0].iov_len = sizeof(struct ether_frame);
    /* Point to frame payload */
    msgvec[1].iov_base = mip_header;
    msgvec[1].iov_len = sizeof(mip_header);

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

    so = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_MIP));
    check(so != -1, "Failed to create raw socket");
    return so;

    error:
        return -1;
}

int complete_mip_arp(struct sockaddr_ll *so_name, int num_interfaces, int raw_socket_fd, uint8_t mip_address){
    int rc = 0;
    int i = 0;
    struct mip_header *request;
    request = create_arp_request_package(mip_address);

    for (i = 0; i < num_interfaces; i++){
        rc = send_raw_mip_packet(raw_socket_fd, &so_name[i],NULL, request);
        check(rc != -1, "Failed to send arp package for interface");
    }

    error:
        return -1;;

}

struct ether_frame *create_response_ethernet_frame(struct ether_frame *request_ethernet){
    struct ether_frame *response = calloc(1, sizeof(struct ether_frame));
    memcpy(response, request_ethernet, sizeof(struct ether_frame));
    memcpy(response->dst_addr, request_ethernet->src_addr, sizeof( uint8_t) * 6);
    memcpy(response->src_addr, request_ethernet->dst_addr, sizeof( uint8_t) * 6);
    return response;
}

