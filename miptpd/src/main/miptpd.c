#include "../lib/miptp_server.h"
#include "../lib/miptp_app_controller.h"

#define MIPTPD_HELP_MESSAGE "miptpd [-h] [-d] <application_socket> <timeout>"


int main(int argc, char *argv[]){
    int rc = 0;
    printf("%s\n", MIPTPD_HELP_MESSAGE);

    MIPTPServer *server = MIPTPServer_create("mipdsock", "appsock", 1, 1000);
    rc = MIPTPServer_init(server);
    check(rc != -1, "Initializion of the MIPTPServer failed, aborting");
    rc = MIPTPServer_run(server);
    check(rc != -1, "MIPTPServer exited unexpectedly");

    return 0;
    
    error:
        log_err("[MIPTP SERVER] Failed unexpectedly");
        return 1;
}