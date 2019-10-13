#ifndef _ROUTING_H
#define _ROUTING_H

#include "../../../../routerd/src/lib/mip_route_table.h"
#include "../server.h"

// Parses the event on the socket, returns a MIPRouteTablePackage on success, NULL when socket disconnected 0 bytes
MIPRouteTablePackage *parse_route_socket_event(int socket_fd);

int broadcast_route_table(MIPDServer *server, MIPRouteTablePackage *table_package);

int recv_route_table_broadcast(MIPDServer *server, MIPPackage *package);

#endif