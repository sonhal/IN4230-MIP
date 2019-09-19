#include <stdint.h>
#include <stdlib.h>		/* free */
#include <stdio.h> 		/* printf */
#include <string.h>		/* memset */
#include "dbg.h"
#include "mip.h"
#include "link.h"

struct mip_arp_cache_entry {
    uint8_t address;
    uint8_t interface[6];
};

struct mip_arp_cache
{
    struct mip_arp_cache_entry entries[64];
    int64_t entry_index;
};


struct mip_arp_cache *create_cache(){
    struct mip_arp_cache *cache;
    cache = calloc(1, sizeof(struct mip_arp_cache));
    return cache;
}


int append_to_cache(struct mip_arp_cache *cache, uint8_t address, uint8_t interface[]){
    struct mip_arp_cache_entry  new_entry = {.address=address};
    memcpy(new_entry.interface, interface, (sizeof(uint8_t) * 6));

    cache->entries[cache->entry_index] = new_entry;
    int64_t tmp = cache->entry_index;
    cache->entry_index += 1;
    return tmp;
}

void print_cache(struct mip_arp_cache *cache){
    int i = 0;
    struct mip_arp_cache_entry entry;

    printf("----- mipd cache -----\n");
    for(i = 0; i < cache->entry_index; i++){
        entry = cache->entries[i];
        printf("cache entry %d\t mip address %d\t interface ", i, entry.address);

        int k = 0;
        for(k = 0; k < 5; k++){
            printf("%d:", entry.interface[k]);
        }
        printf("%d", entry.interface[k]);
        printf("\n");
    }
    printf("-------------------\n");
}
