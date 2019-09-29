
#ifndef _MIP_H
#define _MIP_H
#include <stdint.h>

#define MIP_ADDRESS uint8_t

struct mip_header {
    uint8_t tra: 3;
    uint8_t ttl: 4;
    uint16_t payload_len: 9;
    MIP_ADDRESS dst_addr;
    MIP_ADDRESS src_addr;
} __attribute__((packed));

struct mip_header *create_arp_request_package(MIP_ADDRESS src_addr);

struct mip_header *create_arp_response_package(MIP_ADDRESS src_addr, struct mip_header *arp_request);

struct mip_header *create_transport_package(MIP_ADDRESS src_addr, MIP_ADDRESS dest_addr);

char *mip_header_to_string(struct mip_header *m_header);
#endif