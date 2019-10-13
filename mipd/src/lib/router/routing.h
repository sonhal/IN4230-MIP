#ifndef _ROUTING_H
#define _ROUTING_H

#include "../server.h"

int broadcast_route_table(MIPDServer *server, int socket_fd);

int recv_route_table_broadcast(MIPDServer *server, MIPPackage *package);

#endif