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