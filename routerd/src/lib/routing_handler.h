#ifndef _ROUTING_HANDLER_H
#define _ROUTING_HANDLER_H

#include "mip_route_table.h"
#include "router_server.h"

/*
    Handles broadcasting the MIP route table. Serializies and passes the serialized table to the local MIP daemon
    returns 1 on success, -1 on failure
*/
int broadcast_route_table(MIPRouteTable *table, int socket);

/*
    Handles events on the route socket. Returns a pointer to a neighbor MIP route table
    return NULL on failure
*/
MIPRouteTable *handle_route_request(MIPRouteTable *table, int socket);

// Returns 1 if a new broadcast should be completed, 0 if not
int should_complete_route_broadcast(RouterServer *server);

#endif