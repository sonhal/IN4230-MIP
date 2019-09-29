#include <string.h>
#include "ether_frame.h"


struct ether_frame *create_response_ethernet_frame(struct ether_frame *request_ethernet){
    struct ether_frame *response = calloc(1, sizeof(struct ether_frame));
    memcpy(response, request_ethernet, sizeof(struct ether_frame));
    memcpy(response->dst_addr, request_ethernet->src_addr, sizeof( uint8_t) * 6);
    memcpy(response->src_addr, request_ethernet->dst_addr, sizeof( uint8_t) * 6);
    return response;
}

struct ether_frame *create_transport_ethernet_frame(uint8_t src[], uint8_t dest[]){
    struct ether_frame *frame = calloc(1, sizeof(struct ether_frame));
    memcpy(frame->dst_addr, &dest, sizeof( uint8_t) * 6);
    memcpy(frame->src_addr, &src, sizeof( uint8_t) * 6);
    frame->eth_proto[0] = 0x88;
    frame->eth_proto[1] = 0xB5;
    return frame;
}


struct ether_frame *create_ethernet_frame(int8_t *dest[], struct sockaddr_ll *so_name){
    struct ether_frame *frame = calloc(1, sizeof(struct ether_frame));
    /* Fill in Ethernet header */
    memcpy(frame->dst_addr, dest, MAC_ADDRESS_SIZE);
    memcpy(frame->src_addr, so_name->sll_addr, MAC_ADDRESS_SIZE);
    /* Match the ethertype in packet_so9cket.c: */
    frame->eth_proto[0] = 0x88;
    frame->eth_proto[1] = 0xB5;
    return frame;
}

char *ether_addr_str(int8_t mac_address[6]){
    char *macaddr = calloc(6 * 2 + 6, sizeof(char));
    
    int i = 0;
    for (i = 0; i < 6; i++){
        char *buf = strdup(macaddr);

        sprintf(macaddr, "%s%02hhx%s",
                buf,
                mac_address[i],
                (i < 5) ? ":" : "");

        free(buf);
    }
    return macaddr;
}

char *protocol_str(int8_t e_protocol[2]){
    char *protocol = calloc(4, sizeof(char));
    
    int i = 0;
    for (i = 0; i < 2; i++){
        char *buf = strdup(protocol);

        sprintf(protocol, "%s%02hhx",
                buf,
                e_protocol[i]);

        free(buf);
    }
    return protocol;
}

char *ether_frame_to_string(struct ether_frame *e_frame){
    char *e_string = calloc(1, 256);
    char *src_str = ether_addr_str(e_frame->src_addr);
    char *dst_str = ether_addr_str(e_frame->dst_addr);
    char *e_prot_str = protocol_str(e_frame->eth_proto);
    
    sprintf(e_string, "---- Ethernet frame -----\nproto:\t%s\nsrc:\t%s\ndest:\t%s\n-------------------------\n",
            e_prot_str, src_str, dst_str);
    free(src_str);
    free(dst_str);
    free(e_prot_str);
    return e_string;
}
