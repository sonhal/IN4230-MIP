#ifndef _MIP_ARP_H
#define _MIP_ARP_H

#include <stdint.h>

#include "packaging/mip_package.h"

struct mip_arp_cache_entry {
    MIP_ADDRESS mip_address;
    int src_socket;
    uint8_t dst_interface[6];
    unsigned long last_update;
};

struct mip_arp_cache
{
    struct mip_arp_cache_entry entries[64];
    int size;
    unsigned long update_freq;
    unsigned long last_arp;
};


struct mip_arp_cache *create_cache(long update_freq);

int append_to_cache(struct mip_arp_cache *cache, int src_socket, MIP_ADDRESS mip_address, uint8_t interface[]);

int query_mip_address_src_socket(struct mip_arp_cache *cache, MIP_ADDRESS mip_address);

int query_mip_address_pos(struct mip_arp_cache *cache, MIP_ADDRESS mip_address);

MIP_ADDRESS query_cache_socket_mip_address(struct mip_arp_cache *cache, int sock);

int should_complete_new_arp(struct mip_arp_cache *cache);

int complete_mip_arp(struct interface_table *table, struct mip_arp_cache *cache);

int update_arp_cache(struct interface_table *table, struct mip_arp_cache *cache);

int handle_mip_arp_request(struct mip_arp_cache *cache, MIPPackage *received_package, struct interface_record *i_received_on);

void print_cache(struct mip_arp_cache *cache);

#endif