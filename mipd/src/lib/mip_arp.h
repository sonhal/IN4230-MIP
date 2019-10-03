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
    unsigned long update_freq;
    unsigned long last_update;
};


struct mip_arp_cache *create_cache(long update_freq);

int append_to_cache(struct mip_arp_cache *cache, int src_socket, uint8_t mip_address, uint8_t interface[]);

int query_mip_address_src_socket(struct mip_arp_cache *cache, uint8_t mip_address);

int query_mip_address_pos(struct mip_arp_cache *cache, uint8_t mip_address);

int should_update_cache(struct mip_arp_cache *cache);

int complete_mip_arp(struct interface_table *table, struct mip_arp_cache *cache);

int update_cache_on_freq(struct interface_table *table, struct mip_arp_cache *cache);

void print_cache(struct mip_arp_cache *cache);
