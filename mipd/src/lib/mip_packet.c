#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../../commons/src/dbg.h"

#include "mip_packet.h"


struct mip_packet *create_mip_packet(const struct ether_frame *e_frame, const struct mip_header *m_header, const char *message){
    int rc = 0;
    size_t message_len = strlen(message);
    check(message_len < PAYLOAD_MAX_SIZE, "Payload to large, cannot create MIP packet");

    struct mip_packet *new_packet = calloc(1, sizeof(struct mip_packet));
    memcpy(&new_packet->e_frame, e_frame, sizeof(struct ether_frame));
    memcpy(&new_packet->m_header, m_header, sizeof(struct mip_header));
    strncpy(new_packet->message, message, PAYLOAD_MAX_SIZE - 1);
    new_packet->message[message_len] = '\0';
    return new_packet;

    error:
        destroy_mip_packet(new_packet);
        return NULL;
}

struct mip_packet *create_empty_mip_packet(){
    struct ether_frame *e_frame = calloc(1, sizeof(struct ether_frame));
    struct mip_header* m_header = calloc(1, sizeof(struct mip_header));
    char *message = "\0";
    struct mip_packet *packet = create_mip_packet(e_frame, m_header, message);
    free(e_frame);
    free(m_header);
    return packet;
}

void destroy_mip_packet(struct mip_packet *packet) {
    free(packet);
}