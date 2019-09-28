#include <stdint.h>

#define MIP_ADDRESS uint8_t

struct mip_header {
    unsigned int tra: 3;
    unsigned int ttl: 4;
    unsigned int payload_len: 9;
    MIP_ADDRESS dst_addr;
    MIP_ADDRESS src_addr;
} __attribute__((packed));

struct mip_header *create_arp_request_package(MIP_ADDRESS src_addr);

struct mip_header *create_arp_response_package(MIP_ADDRESS src_addr, struct mip_header *arp_request);

struct mip_header *create_transport_package(MIP_ADDRESS src_addr, MIP_ADDRESS dest_addr);