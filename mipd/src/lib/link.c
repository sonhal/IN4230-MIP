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
#include "packaging/mip_package.h"


#define BUF_SIZE 1600
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


int sendto_raw_mip_package(int sd, struct sockaddr_ll *so_name, MIPPackage *package){
    int rc = 0;

    //Create raw package
    int total_package_size = 0;
    int payload_len_in_bytes = package->m_header.payload_len * MIP_PAYLOAD_WORD;
    total_package_size = (sizeof(MIPPackage) + payload_len_in_bytes);
    BYTE *raw_package = calloc(total_package_size, sizeof(BYTE));
    memcpy(raw_package, package, sizeof(MIPPackage));
    memcpy(&raw_package[sizeof(MIPPackage)], package->message, payload_len_in_bytes);

    /* Send message */
    rc = sendto(sd,raw_package, total_package_size, 0, so_name, sizeof(struct sockaddr_ll));
    check(rc != -1, "Failed to send mip package");

    free(raw_package);
    MIPPackage_destroy(package);
    return 0;

    error:
        if(raw_package)free(raw_package);
        MIPPackage_destroy(package);
        return -1;
}

int recv_raw_mip_package(int sd, MIPPackage *package) {
    check(package != NULL, "Invalid package passed to recv mip package");
    int rc = 0;
    BYTE *raw_package = calloc(1, MIP_PACKAGE_MAX_SIZE);

    rc = recv(sd, raw_package, MIP_PACKAGE_MAX_SIZE, 0);

    // Create a tmp pointer to message as it will be overwritten durring read
    BYTE *tmp_p = package->message;
    // Parse raw package to mip_package
    memcpy(package, raw_package, sizeof(MIPPackage));
    int payload_len_in_bytes = package->m_header.payload_len * MIP_PAYLOAD_WORD;

    memcpy(tmp_p, &raw_package[sizeof(MIPPackage)], payload_len_in_bytes);
    package->message = tmp_p;

    check(rc != -1, "Failed to receive MIP package");
    free(raw_package);
    return rc;

    error:
        if(raw_package) free(raw_package);
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


