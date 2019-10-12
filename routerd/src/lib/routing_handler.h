#ifndef _ROUTING_HANDLER_H
#define _ROUTING_HANDLER_H

#include "mip_route_table.h"
#include "router_server.h"


int broadcast_route_table(MIPRouteTable *table, int socket);

MIPRouteTable *handle_route_request(MIPRouteTable *table, int socket);

// Returns 1 if a new broadcast should be completed, 0 if not
int should_complete_route_broadcast(RouterServer *server);

#endif