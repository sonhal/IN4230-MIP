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
