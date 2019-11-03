#include "../lib/miptp_server.h"
#include "../lib/miptp_app_controller.h"

#define MIPTPD_HELP_MESSAGE "miptpd [-h] [-d] <mipd_socket> <timeout>"


int main(int argc, char *argv[]){
    printf("%s\n", MIPTPD_HELP_MESSAGE);

    MIPTPAppController *controller = MIPTPAppController_create(10);
    printf("max connections: %d", controller->max_connections);
    MIPTPAppController_destroy(controller);

    return 0;
}