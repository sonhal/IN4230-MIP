#include <stdint.h>

struct mip_arp_cache_entry {
    uint8_t address;
    int src_socket;
    uint8_t dst_interface[6];
};

struct mip_arp_cache
{
    struct mip_arp_cache_entry entries[64];
    int size;
};


struct mip_arp_cache *create_cache();

int append_to_cache(struct mip_arp_cache *cache, int src_socket, uint8_t mip_address, uint8_t interface[]);

int query_mip_address_src_socket(struct mip_arp_cache *cache, uint8_t mip_address);

int query_mip_address_pos(struct mip_arp_cache *cache, uint8_t mip_address);

void print_cache(struct mip_arp_cache *cache);
