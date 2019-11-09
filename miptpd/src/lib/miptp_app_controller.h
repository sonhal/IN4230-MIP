#ifndef _MIPTP_APP_CONTROLLER_H
#define _MIPTP_APP_CONTROLLER_H

#include <stdint.h>

#include "../../../commons/src/list.h"
#include "../../../commons/src/definitions.h"

#include "miptp_job.h"

typedef struct AppConnection
{
    uint16_t port: 14;
    int socket;
    MIPTPJob *job;
} AppConnection;


typedef struct MIPTPAppController
{
    List *connections;
    unsigned int max_connections;
    unsigned long timeout;
} MIPTPAppController;


/* Is responsible for handling a new package received from a tp applications over a UNIX domain socket.
returns 1 if it successful, -1 if it fails to handle the package */
int MIPTPAppController_handle_package(MIPTPAppController *controller, int socket, BYTE *s_package);

MIPTPAppController *MIPTPAppController_create(unsigned int max_connections);

void MIPTPAppController_destroy(MIPTPAppController *app_controller);

int MIPTPAppController_handle_connection(MIPTPAppController *app_controller, int epoll_fd, struct epoll_event *event);

int MIPTPAppController_is_a_connection(MIPTPAppController *controller, int socket);

int MIPTPAppController_disconnect(MIPTPAppController *controller, int socket);

int MIPTPAppController_pump(MIPTPAppController *controller);

#endif