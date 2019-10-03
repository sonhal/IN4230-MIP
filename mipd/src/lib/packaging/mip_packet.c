#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "../../../../commons/src/dbg.h"
#include "ether_frame.h"
#include "mip_header.h"
#include "mip_packet.h"



static int create_payload(struct mip_packet *packet, BYTE *message, size_t message_size){
    int number_of_words_needed = calculate_mip_payload_words(message_size);
    check(number_of_words_needed <= PAYLOAD_MAX_WORD_NUM, "Payload to large, cannot create MIP packet");  

    packet->message = calloc(number_of_words_needed, MIP_PAYLOAD_WORD);
    memcpy(packet->message, message, message_size);
    packet->m_header.payload_len = number_of_words_needed;

    return 0;

    error:
        return -1;
}

/*
Important general MIP protocol rule
The payload of a MIP packet is always a multiple of 4, and the total packet size including the 4-byte MIP header must not exceed 1500 bytes.
MIP does not fragment packets, i.e. it cannot accept messages that do not fulfill these constraints.
If a message that does not fulfill these constraints is given to the MIP daemon for sending, this should cause an error and the message should not be sent.

Length
The length of the payload in 32-bit words (i.e. the total length of the MIP packet including the MIP header in bytes is Length*4+4).
*/
struct mip_packet *create_mip_packet(const struct ether_frame *e_frame, const struct mip_header *m_header, const BYTE *message, size_t message_size){
    int rc = 0;
    struct mip_packet *new_packet = calloc(1, sizeof(struct mip_packet));
    
    memcpy(&new_packet->e_frame, e_frame, sizeof(struct ether_frame));
    memcpy(&new_packet->m_header, m_header, sizeof(struct mip_header));
    new_packet->m_header.payload_len = 0;
    

    if(message_size > 0){
       rc = create_payload(new_packet, message, message_size);
       check(rc != -1, "Failed to create payload for new mip packet");
    }
    
    return new_packet;

    error:
        destroy_mip_packet(new_packet);
        return NULL;
}


struct mip_packet *create_mip_packet_from_addrs(int8_t src_mip_addrs, int8_t src_mac_addrs[6] , int8_t dst_mip_addrs, int8_t dst_mac_addrs[6], const BYTE *message, size_t message_size){
    int rc = 0;

    struct mip_header *m_header = create_transport_mip_header(src_mip_addrs, dst_mip_addrs);
    check(m_header != NULL, "Failed to create mip header");
    struct ether_frame *e_frame = create_transport_ethernet_frame(src_mac_addrs, dst_mac_addrs);
    check(e_frame != NULL, "Failed to create ether frame");
    struct mip_packet *new_packet = create_mip_packet(e_frame, m_header, NULL, 0);
    check(new_packet != NULL, "Failed to create mip packet");
    free(e_frame);
    free(m_header);
    
    return new_packet;

    error:
        destroy_mip_packet(new_packet);
        return NULL;
}

struct mip_packet *create_empty_mip_packet(){
    struct ether_frame *e_frame = calloc(1, sizeof(struct ether_frame));
    struct mip_header* m_header = calloc(1, sizeof(struct mip_header));
    BYTE *message = calloc(PAYLOAD_MAX_WORD_NUM, MIP_PAYLOAD_WORD);
    struct mip_packet *packet = create_mip_packet(e_frame, m_header, message, PAYLOAD_MAX_WORD_NUM * MIP_PAYLOAD_WORD);
    destroy_ether_frame(e_frame);
    destroy_mip_header(m_header);
    return packet;
}


struct mip_packet *create_mip_arp_request_packet(uint8_t src_mip_addrs, uint8_t *src_mac_addrs){
    struct mip_header *request_m_header = create_arp_request_mip_header(src_mip_addrs);
    struct ether_frame *request_e_frame = create_ethernet_arp_frame_from_mac_addrs(src_mac_addrs);
    struct mip_packet *request_m_packet = create_mip_packet(request_e_frame, request_m_header, NULL, 0);
    destroy_ether_frame(request_e_frame);
    destroy_mip_header(request_m_header);
    return request_m_packet;
}

char *mip_packet_to_string(struct mip_packet *packet){
    char *m_p_string = calloc(20, sizeof(char));

    char *e_frame_str = ether_frame_to_string(&packet->e_frame);
    char *m_header_str = mip_header_to_string(&packet->m_header);

    sprintf(m_p_string, "-------- MIP packet --------\n%s%s-------- MIP Packet END --------\n",
            e_frame_str, m_header_str, packet->message);

    return m_p_string;
}


/*
The length of the payload in 32-bit words (i.e. the total length of the MIP packet including the MIP header in bytes is Length*4+4).
*/
int calculate_mip_payload_words(size_t message_size){
    int number_of_words = 0;
    number_of_words = message_size / MIP_PAYLOAD_WORD;
    if((message_size % MIP_PAYLOAD_WORD != 0)){
        number_of_words++;
    }
    return number_of_words;
}


void destroy_mip_packet(struct mip_packet *packet) {
    if(packet->message)free(packet->message);
    free(packet);
}