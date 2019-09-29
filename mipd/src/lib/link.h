
#ifndef _LINK_H
#define _LINK_H
#include <linux/if_packet.h>	/* AF_PACKET */
#include "mip.h"
#include "ether_frame.h"
#include "mip_packet.h"


char *macaddr_str(struct sockaddr_ll *sa);

int last_inteface(struct sockaddr_ll *so_name);

int collect_intefaces(struct sockaddr_ll *so_name, int buffer_n);

int receive_raw_mip_packet(int sd, struct ether_frame  *frame_hdr, struct sockaddr_ll *so_name, struct mip_header *header);

int send_raw_mip_packet(int sd, struct sockaddr_ll *so_name, struct ether_frame *frame_hdr, struct mip_header *mip_header);

int sendto_raw_mip_packet(int sd, struct sockaddr_ll *so_name, struct mip_packet *packet);

int recv_raw_mip_packet(int sd, struct mip_packet *packet);

int setup_raw_socket();

int complete_mip_arp(struct interface_table *table);

#endif