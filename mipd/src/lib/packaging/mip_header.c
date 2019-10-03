#include <stdlib.h>		/* free */
#include <stdio.h> 		/* printf */
#include <string.h>		/* memset */
#include <stdint.h>
#include "../../../../commons/src/dbg.h"
#include "mip_header.h"


// Returns heap allocated MIP header ready for ARP
struct mip_header *create_arp_request_mip_header(MIP_ADDRESS src_addr){
    struct mip_header *header;
    header = calloc(1, sizeof(struct mip_header));
    header->tra = 1;
    header->ttl = 15;
    header->payload_len = 0;
    header->dst_addr = 255;
    header->src_addr = src_addr;

    return header;
}


// Returns heap allocated MIP header ready for use as a response to a ARP request
struct mip_header *create_arp_response_mip_header(MIP_ADDRESS src_addr, struct mip_header *arp_request){
    struct mip_header *header;
    check(arp_request != NULL, "Invalid argument, arp_request was NULL");
    header = create_arp_request_mip_header(src_addr);
    header->tra = 0;
    header->dst_addr = arp_request->src_addr;
    return header;

    error:
        exit("Failure");
        return NULL;
}


// Returns a transport mip header
struct mip_header *create_transport_mip_header(MIP_ADDRESS src_addr, MIP_ADDRESS dest_addr){
    struct mip_header *header;
    header = create_arp_request_mip_header(src_addr);
    header->tra = 3;
    header->dst_addr = dest_addr;
    return header;
}

char *mip_header_to_string(struct mip_header *m_header){
    char *m_string = calloc(1, 256);

    sprintf(m_string, "---- MIP header ----\ntra:\t%d\nttl:\t%d\npayload len:\t%d\ndst addres:\t%d\nsrc address:\t%d\n--------------------\n",
            m_header->tra, m_header->ttl, m_header->payload_len, m_header->dst_addr, m_header->src_addr);

    return m_string;
}


void destroy_mip_header(struct mip_header *header){
    if(header)free(header);
}