#ifndef _MIPTP_SERVER_H
#define _MIPTP_SERVER_H

#include <sys/epoll.h> 

#include "../../../commons/src/bound_socket.h"

#include "miptp_app_controller.h"

typedef struct MIPTPServer {
    unsigned short int debug_active;
    char *mipd_socket;
    int mipd_fd;
    BoundSocket *app_connections_socket;
    unsigned long timeout;
    int epoll_fd;
    struct epoll_event *event_buffer;
    MIPTPAppController *app_controller;
} MIPTPServer;


MIPTPServer *MIPTPServer_create(char *mipd_socket, char *app_connections_socket, unsigned short int debug_active, unsigned long timeout);

void MIPTPServer_destroy(MIPTPServer *server);

int MIPTPServer_init(MIPTPServer *server);

// Starts the server, function will block until the server is stoped, returns 1 if server exited normally
int MIPTPServer_run(MIPTPServer *server);

#define MIPTPServer_is_debug_active(S) ((S)->debug_active ? 1 : 0)

#define MIPTPServer_log(S, M, ...) if(MIPTPServer_is_debug_active(S)) printf("[MIPTP SERVER] " M "\n", ##__VA_ARGS__)

#endif