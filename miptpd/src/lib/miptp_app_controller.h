#ifndef _MIPTP_APP_CONTROLLER_H
#define _MIPTP_APP_CONTROLLER_H

#include <stdint.h>

#include "../../../commons/src/list.h"
#include "../../../commons/src/definitions.h"

typedef struct MIPTPAppController
{
    List *connections;
    unsigned int max_connections;

} MIPTPAppController;

/*
Is responsible for handling a new package received from a tp applications over a UNIX domain socket.
returns 1 if it successful, -1 if it fails to handle the package
*/
int MIPTPAppController_handle_new_package(BYTE package);

MIPTPAppController *MIPTPAppController_create(unsigned int max_connections);

void MIPTPAppController_destroy(MIPTPAppController *app_controller);



#endif