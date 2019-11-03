
#ifndef _MIP_H
#define _MIP_H
#include <stdint.h>
#include "../../../../commons/src/definitions.h"

struct mip_header {
    uint8_t tra: 3;
    uint8_t ttl: 4;
    uint16_t payload_len: 9;
    MIP_ADDRESS dst_addr;
    MIP_ADDRESS src_addr;
}__attribute__((packed));

struct mip_header *create_arp_request_mip_header(MIP_ADDRESS src_addr);

struct mip_header *create_route_broadcast_mip_header(MIP_ADDRESS src_addr);

struct mip_header *create_arp_response_mip_header(MIP_ADDRESS src_addr, struct mip_header *arp_request);

struct mip_header *create_transport_mip_header(MIP_ADDRESS src_addr, MIP_ADDRESS dest_addr);

char *mip_header_to_string(struct mip_header *m_header);

void destroy_mip_header(struct mip_header *header);
#endif