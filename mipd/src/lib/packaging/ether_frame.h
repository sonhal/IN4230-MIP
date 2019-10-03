#ifndef _ETHER_FRAME_H
#define _ETHER_FRAME_H

#include <linux/if_packet.h>
#include <net/ethernet.h> /* the L2 protocols */
#include <stdint.h>

#define ETH_BROADCAST_ADDR {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
#define MAC_ADDRESS_SIZE (6 * sizeof(uint8_t))

struct ether_frame {
    uint8_t dst_addr[6];
    uint8_t src_addr[6];
    uint8_t eth_proto[2];
    uint8_t contents[0];
} __attribute__((packed));

struct ether_frame *create_response_ethernet_frame(struct ether_frame *request_ethernet);

struct ether_frame *create_transport_ethernet_frame(uint8_t src[], uint8_t dest[]);

struct ether_frame *create_ethernet_frame(int8_t *dest[], struct sockaddr_ll *so_name);

struct ether_frame *create_ethernet_arp_frame(struct sockaddr_ll *so_name);

struct ether_frame *create_ethernet_arp_frame_from_mac_addrs(uint8_t *mac_address);

char *ether_frame_to_string(struct ether_frame *e_frame);

void destroy_ether_frame(struct ether_frame *frame);
#endif