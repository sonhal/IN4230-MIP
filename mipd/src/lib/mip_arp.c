#include <stdint.h>
#include <stdlib.h>		/* free */
#include <stdio.h> 		/* printf */
#include <string.h>		/* memset */
#include "../../../commons/src/dbg.h"
#include "packaging/mip_header.h"
#include "link.h"
#include "mip_arp.h"
#include <time.h>

static long get_milli() {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (long)ts.tv_sec * 1000L + (ts.tv_nsec / 1000000L);
}

struct mip_arp_cache *create_cache(long update_freq){
    struct mip_arp_cache *cache;
    cache = calloc(1, sizeof(struct mip_arp_cache));
    cache->update_freq = update_freq;
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

int should_update_cache(struct mip_arp_cache *cache){
    long current_time = get_milli();
    long last_update = cache->last_update;
    if(current_time - last_update > cache->update_freq) return 1;
    return 0;
}


int complete_mip_arp(struct interface_table *table, struct mip_arp_cache *cache){
    int rc = 0;
    int i = 0;
    struct mip_header *request_m_header;
    struct ether_frame *request_e_frame;
    struct mip_packet *request_m_packet;
    uint8_t broadcast_addr[] = ETH_BROADCAST_ADDR;
    
    for (i = 0; i < table->size; i++){
        int mip_addr = table->interfaces[i].mip_address;
        int socket = table->interfaces[i].raw_socket;
        struct sockaddr_ll *so_name = table->interfaces[i].so_name;
        int8_t *mac_addr = &table->interfaces[i].interface;

        request_m_packet = create_mip_arp_request_packet(mip_addr, mac_addr);
        rc = sendto_raw_mip_packet(socket, so_name, request_m_packet);
        check(rc != -1, "Failed to send arp package for interface");
    }

    // Update cache with the current time
    cache->last_update = get_milli();
    return 1;

    error:
        return -1;;

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
