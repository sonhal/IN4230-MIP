#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>


#include "../../../../commons/src/dbg.h"
#include "ether_frame.h"
#include "mip_header.h"
#include "mip_package.h"


static int create_payload(MIPPackage *package, BYTE *message, size_t message_size){
    int number_of_words_needed = calculate_mip_payload_words(message_size);
    check(number_of_words_needed <= PAYLOAD_MAX_WORD_NUM, "Payload to large, cannot create MIP package");  

    package->message = calloc(number_of_words_needed, MIP_PAYLOAD_WORD);
    memcpy(package->message, message, message_size);
    package->m_header.payload_len = number_of_words_needed;

    return 0;

    error:
        return -1;
}


/*
Important general MIP protocol rule
The payload of a MIP package is always a multiple of 4, and the total package size including the 4-byte MIP header must not exceed 1500 bytes.
MIP does not fragment packages, i.e. it cannot accept messages that do not fulfill these constraints.
If a message that does not fulfill these constraints is given to the MIP daemon for sending, this should cause an error and the message should not be sent.

Length
The length of the payload in 32-bit words (i.e. the total length of the MIP package including the MIP header in bytes is Length*4+4).
*/
MIPPackage *MIPPackage_create(const struct ether_frame *e_frame, const struct mip_header *m_header, const BYTE *message, size_t message_size){
    int rc = 0;
    MIPPackage *new_package = calloc(1, sizeof(MIPPackage));
    
    memcpy(&new_package->e_frame, e_frame, sizeof(struct ether_frame));
    memcpy(&new_package->m_header, m_header, sizeof(struct mip_header));
    new_package->m_header.payload_len = 0;
    

    if(message_size > 0){
       rc = create_payload(new_package, message, message_size);
       check(rc != -1, "Failed to create payload for new mip package");
    }
    
    return new_package;

    error:
        MIPPackage_destroy(new_package);
        return NULL;
}


MIPPackage *MIPPackage_create_from_addrs(int8_t src_mip_addrs, int8_t *src_mac_addrs , int8_t dst_mip_addrs, int8_t *dst_mac_addrs, const BYTE *message, size_t message_size) {
    int rc = 0;

    struct mip_header *m_header = create_transport_mip_header(src_mip_addrs, dst_mip_addrs);
    check(m_header != NULL, "Failed to create mip header");
    struct ether_frame *e_frame = create_transport_ethernet_frame(src_mac_addrs, dst_mac_addrs);
    check(e_frame != NULL, "Failed to create ether frame");
    MIPPackage *new_package = MIPPackage_create(e_frame, m_header, NULL, 0);
    check(new_package != NULL, "Failed to create mip package");
    free(e_frame);
    free(m_header);
    
    return new_package;

    error:
        MIPPackage_destroy(new_package);
        return NULL;
}

MIPPackage *MIPPackage_create_raw(int8_t src_mip_addrs, int8_t *src_mac_addrs , int8_t dst_mip_addrs, int8_t *dst_mac_addrs, const BYTE *message, size_t message_size, uint8_t tra) {
    int rc = 0;
    check(tra < 5, "Invalid tra");

    struct mip_header *m_header = create_transport_mip_header(src_mip_addrs, dst_mip_addrs);
    m_header->tra = tra;
    check(m_header != NULL, "Failed to create mip header");

    struct ether_frame *e_frame = create_transport_ethernet_frame(src_mac_addrs, dst_mac_addrs);
    check(e_frame != NULL, "Failed to create ether frame");
    MIPPackage *new_package = MIPPackage_create(e_frame, m_header, message, message_size);
    check(new_package != NULL, "Failed to create mip package");
    free(e_frame);
    free(m_header);
    
    return new_package;

    error:
        MIPPackage_destroy(new_package);
        return NULL;
}

MIPPackage *MIPPackage_create_empty() {
    struct ether_frame *e_frame = calloc(1, sizeof(struct ether_frame));
    struct mip_header* m_header = calloc(1, sizeof(struct mip_header));
    BYTE *message = calloc(PAYLOAD_MAX_WORD_NUM, MIP_PAYLOAD_WORD);
    MIPPackage *package = MIPPackage_create(e_frame, m_header, message, PAYLOAD_MAX_WORD_NUM * MIP_PAYLOAD_WORD);
    destroy_ether_frame(e_frame);
    destroy_mip_header(m_header);
    return package;
}


MIPPackage *MIPPackage_create_mip_arp_request_packet(uint8_t src_mip_addrs, uint8_t *src_mac_addrs) {
    struct mip_header *request_m_header = create_arp_request_mip_header(src_mip_addrs);
    struct ether_frame *request_e_frame = create_ethernet_arp_frame_from_mac_addrs(src_mac_addrs);
    MIPPackage *request_m_package = MIPPackage_create(request_e_frame, request_m_header, NULL, 0);
    destroy_ether_frame(request_e_frame);
    destroy_mip_header(request_m_header);
    return request_m_package;
}


char *MIPPackage_to_string(MIPPackage *package) {
    char *e_frame_str = ether_frame_to_string(&package->e_frame);
    char *m_header_str = mip_header_to_string(&package->m_header);
    char *format = "-------- MIP package --------\n%s%s-------- MIP Package END --------\n";
    int str_len = strlen(e_frame_str) + strlen(m_header_str) + strlen(format);
    char *m_p_string = calloc(str_len, sizeof(char));

    sprintf(m_p_string, "-------- MIP package --------\n%s%s-------- MIP Package END --------\n",
            e_frame_str, m_header_str, package->message);

    return m_p_string;
}


/*
The length of the payload in 32-bit words (i.e. the total length of the MIP package including the MIP header in bytes is Length*4+4).
*/
int calculate_mip_payload_words(size_t message_size){
    int number_of_words = 0;
    number_of_words = round(message_size / MIP_PAYLOAD_WORD);
    if((message_size % MIP_PAYLOAD_WORD != 0)){
        number_of_words++;
    }
    return number_of_words;
}


void MIPPackage_destroy(MIPPackage *package) {
    if(package){
        if(package->message)free(package->message);
        free(package);
    }
}