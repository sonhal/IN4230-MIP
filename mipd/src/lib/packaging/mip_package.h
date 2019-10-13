#ifndef _MIPPackage_H
#define _MIPPackage_H

#include <stdlib.h>
#include <stdlib.h>

#include "ether_frame.h"
#include "mip_header.h"

#define PAYLOAD_MAX_SIZE_BYTES 1496
#define PAYLOAD_MAX_WORD_NUM 374
#define MIP_PAYLOAD_WORD (sizeof(uint8_t) * 4)
#define BYTE uint8_t

#define MIP_PACKAGE_MAX_SIZE (188 * sizeof(BYTE))

struct MIPPackage{
    struct ether_frame e_frame;
    struct mip_header m_header;
    BYTE *message;
}__attribute__((packed));

typedef struct MIPPackage MIPPackage;

 #define mip_header_payload_length_in_bytes(P) P->m_header.payload_len * (sizeof(uint8_t) * 4)

MIPPackage *MIPPackage_create(const struct ether_frame *e_frame, const struct mip_header *m_header, const BYTE *message, size_t message_size);

MIPPackage *MIPPackage_create_from_addrs(int8_t src_mip_addrs, int8_t *src_mac_addrs , int8_t dst_mip_addrs, int8_t *dst_mac_addrs, const BYTE *message, size_t message_size);

MIPPackage *MIPPackage_create_mip_arp_request_packet(uint8_t src_mip_addrs, uint8_t *src_mac_addrs);

MIPPackage *MIPPackage_create_raw(int8_t src_mip_addrs, int8_t *src_mac_addrs , int8_t dst_mip_addrs, int8_t *dst_mac_addrs, const BYTE *message, size_t message_size, uint8_t tra);

MIPPackage *MIPPackage_create_empty();

void MIPPackage_destroy(MIPPackage *package);

char *MIPPackage_to_string(MIPPackage *package);

int calculate_mip_payload_words(size_t message_size);

int calculate_mip_payload_padding(size_t message_size);

#endif