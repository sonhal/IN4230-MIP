
#ifndef _LINK_H
#define _LINK_H
#include <linux/if_packet.h>	/* AF_PACKET */
#include "mip_header.h"
#include "ether_frame.h"
#include "mip_packet.h"


int last_inteface(struct sockaddr_ll *so_name);

int sendto_raw_mip_packet(int sd, struct sockaddr_ll *so_name, struct mip_packet *packet);

int recv_raw_mip_packet(int sd, struct mip_packet *packet);

int setup_raw_socket();

int complete_mip_arp(struct interface_table *table);

#endif