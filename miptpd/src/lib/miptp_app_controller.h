#ifndef _MIPTP_APP_CONTROLLER_H
#define _MIPTP_APP_CONTROLLER_H

#include <stdint.h>

#include "../../../commons/src/list.h"
#include "../../../commons/src/definitions.h"
#include "../../../commons/src/client_package.h"

#include "miptp_send_job.h"
#include "miptp_receive_job.h"


typedef struct MIPTPAppController
{
    List *connections;
    unsigned int max_connections;
    unsigned long timeout;
    int mipd_socket;
} MIPTPAppController;

MIPTPAppController *MIPTPAppController_create(int mipd_socket, unsigned int max_connections);

void MIPTPAppController_destroy(MIPTPAppController *app_controller);

/* Is responsible for handling a new package received from a tp applications over a UNIX domain socket.
returns 1 if it successful, -1 if it fails to handle the package */
int MIPTPAppController_handle_app_package(MIPTPAppController *controller, int socket, BYTE *s_package);

int MIPTPAppController_handle_mipd_package(MIPTPAppController *app_controller, int mipd_socket, BYTE *s_package);

int MIPTPAppController_handle_connection(MIPTPAppController *app_controller, int epoll_fd, struct epoll_event *event);

int MIPTPAppController_is_a_connection(MIPTPAppController *controller, int socket);

int MIPTPAppController_disconnect(MIPTPAppController *controller, int socket);

int MIPTPAppController_handle_outgoing(MIPTPAppController *controller);

int MIPTPAppController_handle_completes(MIPTPAppController *controller);

int MIPTPAppController_send_MIPTPPackage(MIPTPAppController *controller, int socket, MIPTPPackage *package);

int MIPTPAppController_send_ClientPackage(MIPTPAppController *controller, int socket, ClientPackage *package);

#endif