#include <stdlib.h>		/* free */
#include <stdio.h> 		/* printf */
#include <string.h>		/* memset */
#include <sys/socket.h>		/* socket */
#include <linux/if_packet.h>	/* AF_PACKET */
#include <net/ethernet.h>	/* ETH_* */
#include <arpa/inet.h>		/* htons */
#include <ifaddrs.h>		/* getifaddrs */

#include "interface.h"
#include "../../../commons/src/dbg.h"
#include "link.h"
#include "packaging/mip_packet.h"


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


int sendto_raw_mip_packet(int sd, struct sockaddr_ll *so_name, struct mip_packet *packet){
    int rc = 0;

    //Create raw packet
    int total_packet_size = 0;
    int payload_len_in_bytes = packet->m_header.payload_len * MIP_PAYLOAD_WORD;
    total_packet_size = (sizeof(struct mip_packet) + payload_len_in_bytes);
    BYTE *raw_packet = calloc(total_packet_size, sizeof(BYTE));
    memcpy(raw_packet, packet, sizeof(struct mip_packet));
    memcpy(raw_packet[sizeof(struct mip_packet)], packet->message, payload_len_in_bytes);


    /* Send message */
    rc = sendto(sd,raw_packet, total_packet_size, 0, so_name, sizeof(struct sockaddr_ll));
    check(rc != -1, "Failed to send mip package");

    destroy_mip_packet(packet);
    return 0;

    error:
        destroy_mip_packet(packet);
        return -1;
}

int recv_raw_mip_packet(int sd, struct mip_packet *packet){
    int rc = 0;
    BYTE *raw_packet = calloc(188, sizeof(BYTE));

    rc = recv(sd, raw_packet, 188, 0);

    // Parse raw packet to mip_packet
    memcpy(packet, raw_packet, sizeof(struct mip_packet));
    int payload_len_in_bytes = packet->m_header.payload_len * MIP_PAYLOAD_WORD;
    memcpy(packet->message, raw_packet[sizeof(struct mip_packet)], payload_len_in_bytes);

    check(rc != -1, "Failed to receive MIP packet");
    return rc;

    error:
        return -1;
}


int setup_raw_socket(){
    int so = 0;

    so = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_MIP));
    check(so != -1, "Failed to create raw socket");
    return so;

    error:
        return -1;
}

int complete_mip_arp(struct interface_table *table){
    int rc = 0;
    int i = 0;
    struct mip_header *request_m_header;
    struct ether_frame *request_e_frame;
    struct mip_packet *request_m_packet;
    uint8_t broadcast_addr[] = ETH_BROADCAST_ADDR;
    
    debug("interface table size: %d", table->size);

    for (i = 0; i < table->size; i++){
        int mip_addr = table->interfaces[i].mip_address;
        int socket = table->interfaces[i].raw_socket;
        struct sockaddr_ll *so_name = table->interfaces[i].so_name;
        int8_t mac_addr = table->interfaces[i].interface;

        request_m_header = create_arp_request_package(mip_addr);
        request_e_frame = create_ethernet_frame(&broadcast_addr, &mac_addr);
        request_m_packet = create_mip_packet(request_e_frame, request_m_header, NULL, 0);
        rc = sendto_raw_mip_packet(socket, so_name, request_m_packet);
        check(rc != -1, "Failed to send arp package for interface");
    }
    return 1;

    error:
        return -1;;

}
