#include "forwarding_handler.h"

#include "../../../commons/src/dbg.h"


#define FORWARDING_PACKET_BYTE_SIZE sizeof(MIPForwardRequest)

MIPForwardRequest *recv_forwarding_request(int socket){
    int rc = 0;

    BYTE *raw_packet = calloc(1, FORWARDING_PACKET_BYTE_SIZE);
    MIPForwardRequest *forwarding_request = calloc(1, FORWARDING_PACKET_BYTE_SIZE);

    rc = recv(socket, raw_packet, FORWARDING_PACKET_BYTE_SIZE, 0);
    check(rc != -1, "Failed to recv forwarding request");
    check(rc != 0, "mipd disconnected");

    // Parse raw packet
    memcpy(forwarding_request, raw_packet, FORWARDING_PACKET_BYTE_SIZE);
    free(raw_packet);

    return forwarding_request;

    error:
        return NULL;
}


MIP_ADDRESS handle_forwarding_request(MIPRouteTable *table, int socket){

    MIPForwardRequest *forwarding_request = recv_forwarding_request(socket);
    check(forwarding_request != NULL, "Failed to receive forwarding request");
    printf("Request for next hop to mip address: %d\n", forwarding_request->destination);
    check(forwarding_request->destination >= 0 && forwarding_request->destination < 256, "Invalid destination MIP address");

    MIP_ADDRESS next_hop =  MIPRouteTable_get_next_hop(table, forwarding_request->destination);

    free(forwarding_request);
    return next_hop;

    error:
        return 255;
}


int send_forwarding_response(int socket, MIP_ADDRESS next_hop){
    int rc = 0;

    MIPForwardRequest *forwarding_request = calloc(1, FORWARDING_PACKET_BYTE_SIZE);
    forwarding_request->destination = next_hop;

    rc = send(socket, forwarding_request, FORWARDING_PACKET_BYTE_SIZE, 0);
    check(rc != -1, "Failed to send forwarding response");

    free(forwarding_request);
    return rc;

    error:
        return -1;
}
