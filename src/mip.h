#include <stdint.h>

struct mip_header *create_arp_request_package(uint8_t src_addr);

struct mip_header *create_arp_response_package(int src_addr, struct mip_header *arp_request);