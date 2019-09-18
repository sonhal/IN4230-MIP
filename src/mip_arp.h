#include <stdint.h>

struct mip_arp_cache *create_cache();

int append_to_cache(struct mip_arp_cache *cache, uint8_t address, uint8_t interface[]);

void print_cache(struct mip_arp_cache *cache);