#ifndef _MIP_PACKET_H
#define _MIP_PACKET_H

#include <stdlib.h>

#include "link.h"
#include "mip.h"

#define PAYLOAD_MAX_SIZE 1496

struct mip_packet
{
    struct ether_frame e_frame;
    struct mip_header m_header;
    char message[PAYLOAD_MAX_SIZE];
}__attribute__((packed));

struct mip_packet *create_mip_packet(const struct ether_frame *e_frame, const struct mip_header *m_header, const char *message);

struct mip_packet *create_empty_mip_packet();

void destroy_mip_packet(struct mip_packet *packet);

char *mip_packet_to_string(struct mip_packet *packet);
#endif