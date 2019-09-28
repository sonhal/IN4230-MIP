#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mip_packet.h"


struct mip_packet *create_mip_packet(const struct ether_frame *e_frame, const struct mip_header *m_header, const char *message){
    struct mip_packet *new_packet = calloc(1, sizeof(struct mip_packet));
    memcpy(&new_packet->e_frame, e_frame, sizeof(struct ether_frame));
    memcpy(&new_packet->m_header, m_header, sizeof(struct mip_header));
    new_packet->message = calloc(strlen(message), sizeof(char));
    strncpy(new_packet->message, message, strlen(message));
    new_packet->message_size = strlen(message) * sizeof(char);
    return new_packet;
}

void destroy_mip_packet(struct mip_packet *packet) {
    free(packet->message);
    free(packet);
}