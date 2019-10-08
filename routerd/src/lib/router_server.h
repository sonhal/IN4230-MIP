#ifndef _H_ROUTER_SERVER_H
#define _H_ROUTER_SERVER_H

#include "cli.h"
#include "mip_route_table.h"

// Server Controller struct, represents internal state in the server
typedef struct RouterServer
{
   int debug_active;
   MIPRouteTable *table;
   long broadcast_freq_milli;
   long last_broadcast_milli;
   int epoll_fd;
   int routing_fd;
   char *routing_domain_sock;
   int forward_fd;
   char *forward_domain_sock;
} RouterServer;

// Creates a new Router server, function sets up epoll
RouterServer  *RouterServer_create(RouterdConfig *config);

void RouterServer_destroy(RouterServer *server);

// Setup the socket connections and the epoll connection, must be ran before RouterServer_run
int RouterServer_init(RouterServer *server);

// Starts the server, function will block until the server is stoped, returns 0 if server exited normally
int RouterServer_run(RouterServer *server);

#define RouterServer_is_debug_active(S) ((S)->debug_active ? 1 : 0)

#define RouterServer_log(S, M, ...) if(RouterServer_is_debug_active(S)) printf("[ROUTERD] " M "\n", ##__VA_ARGS__)
#endif
