#include "../../../commons/src/dbg.h"

#include "client_package.h"
#include "miptp_app_controller.h"

int MIPTPAppController_handle_new_package(BYTE s_package){
    ClientPackage *package = ClientPackage_deserialize(s_package);


}

MIPTPAppController *MIPTPAppController_create(unsigned int max_connections){
    MIPTPAppController *controller = calloc(1, sizeof(MIPTPAppController));
    check_mem(controller);
    controller->connections = List_create();
    controller->max_connections = max_connections;

    return controller;

    error:
        log_err("Failed to create MIPTPAppController in MIPTPAppController_create");
        return NULL;
}

void MIPTPAppController_destroy(MIPTPAppController *app_controller){
    if(app_controller){
        if(app_controller->connections){
            List_destroy(app_controller->connections);
        }
        free(app_controller);
        app_controller = NULL;
    }
}