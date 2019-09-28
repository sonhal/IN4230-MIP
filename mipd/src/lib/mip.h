#include <stdint.h>

struct mip_header {
    unsigned int tra: 3;
    unsigned int ttl: 4;
    unsigned int payload_len: 9;
    uint8_t dst_addr;
    uint8_t src_addr;
} __attribute__((packed));

struct mip_header *create_arp_request_package(uint8_t src_addr);

struct mip_header *create_arp_response_package(uint8_t src_addr, struct mip_header *arp_request);

struct mip_header *create_transport_package(uint8_t src_addr, uint8_t dest_addr);