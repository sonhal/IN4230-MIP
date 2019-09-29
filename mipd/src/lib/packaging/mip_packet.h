#ifndef _MIP_PACKET_H
#define _MIP_PACKET_H

#include <stdlib.h>
#include <stdlib.h>

#include "ether_frame.h"
#include "mip_header.h"

#define PAYLOAD_MAX_SIZE 1496
#define PAYLOAD_MAX_WORD_NUM 46
#define MIP_PAYLOAD_WORD (sizeof(uint8_t) * 4)
#define BYTE uint8_t

struct mip_packet
{
    struct ether_frame e_frame;
    struct mip_header m_header;
    BYTE *message;
}__attribute__((packed));

 #define mip_header_payload_length_in_bytes(MIP_PACKET) MIP_PACKET->m_header.payload_len * (sizeof(uint8_t) * 4)

struct mip_packet *create_mip_packet(const struct ether_frame *e_frame, const struct mip_header *m_header, const BYTE *message, size_t message_size);

struct mip_packet *create_empty_mip_packet();

void destroy_mip_packet(struct mip_packet *packet);

char *mip_packet_to_string(struct mip_packet *packet);

int calculate_mip_payload_words(size_t message_size);

int calculate_mip_payload_padding(size_t message_size);

#endif