#include <stdint.h>
#include <stdlib.h>		/* free */
#include <stdio.h> 		/* printf */
#include <string.h>		/* memset */
#include "../../../commons/src/dbg.h"
#include "mip.h"
#include "link.h"
#include "mip_arp.h"

struct mip_arp_cache *create_cache(){
    struct mip_arp_cache *cache;
    cache = calloc(1, sizeof(struct mip_arp_cache));
    return cache;
}


int append_to_cache(struct mip_arp_cache *cache, int src_socket, uint8_t mip_address, uint8_t interface[]){
    struct mip_arp_cache_entry  new_entry = {.address=mip_address, .src_socket=src_socket};
    memcpy(new_entry.dst_interface, interface, (sizeof(uint8_t) * 6));
    cache->entries[cache->size] = new_entry;
    cache->size++;
    return cache->size;
}


// return socket mip address can be reached trough if it is in the cache, -1 if it does not exist in the cache
int query_mip_address_src_socket(struct mip_arp_cache *cache, uint8_t mip_address){
    int rc = 0;
    int i = 0;
    for (i = 0; i < cache->size; i++){
        if(mip_address == cache->entries[i].address){
            return cache->entries[i].src_socket;
        }
    }
    return -1;
}

// return socket mip address can be reached trough if it is in the cache, -1 if it does not exist in the cache
int query_mip_address_pos(struct mip_arp_cache *cache, uint8_t mip_address){
    int rc = 0;
    int i = 0;
    for (i = 0; i < cache->size; i++){
        if(mip_address == cache->entries[i].address){
            return i;
        }
    }
    return -1;
}

void print_cache(struct mip_arp_cache *cache){
    int i = 0;
    struct mip_arp_cache_entry entry;

    printf("----- mipd cache -----\n");
    for(i = 0; i < cache->size; i++){
        entry = cache->entries[i];
        printf("cache entry %d\t mip address %d\t interface \tsrc_socet %d", i, entry.address, entry.src_socket);

        int k = 0;
        for(k = 0; k < 5; k++){
            printf("%d:", entry.dst_interface[k]);
        }
        printf("%d", entry.dst_interface[k]);
        printf("\n");
    }
    printf("-------------------\n");
}
