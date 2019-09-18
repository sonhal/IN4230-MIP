#include <stdlib.h>		/* free */
#include <stdio.h> 		/* printf */
#include <string.h>		/* memset */
#include <stdint.h>
#include "dbg.h"

struct mip_header {
    unsigned int tra: 3;
    unsigned int ttl: 4;
    unsigned int payload_len: 9;
    uint8_t dst_addr;
    uint8_t src_addr;
} __attribute__((packed));


// Returns heap allocated MIP header ready for ARP
struct mip_header *create_arp_request_package(uint8_t src_addr){
    struct mip_header *header;
    header = calloc(1, sizeof(struct mip_header));
    header->tra = 1;
    header->ttl = 32;
    header->payload_len = 0;
    header->dst_addr = 255;
    header->src_addr = src_addr;

    return header;
}

// Returns heap allocated MIP header ready for use as a response to a ARP request
struct mip_header *create_arp_response_package(int src_addr, struct mip_header *arp_request){
    struct mip_header *header;
    check(arp_request != NULL, "Invalid argument, arp_request was NULL");
    header = create_arp_request_package(src_addr);
    header->dst_addr = arp_request->src_addr;
    return header;

    error:
        exit("Failure");
        return NULL;
}