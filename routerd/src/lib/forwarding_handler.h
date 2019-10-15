#ifndef _FORWARDING_HANDLER_H
#define _FORWARDING_HANDLER_H

#include <stdlib.h>		/* free */
#include <stdio.h> 		/* printf */
#include <string.h>		/* memset */
#include <sys/socket.h>		/* socket */
#include <linux/if_packet.h>	/* AF_PACKET */
#include <net/ethernet.h>	/* ETH_* */
#include <arpa/inet.h>		/* htons */
#include <ifaddrs.h>		/* getifaddrs */


#include "../../../commons/src/definitions.h"
#include "router_server.h"

typedef struct MIPForwardRequest {
    MIP_ADDRESS destination;
} MIPForwardRequest;

// Handles a request for a Routing table lookup, returns the MIP address of the next hop to the destination MIP address, return 255 on failure
MIP_ADDRESS handle_forwarding_request(MIPRouteTable *table, int socket);

// Sends the result of a forwarding lookup back to the local MIP daemon,
// returns 1 on success, -1 on failure
int send_forwarding_response(int socket, MIP_ADDRESS next_hop);

#endif