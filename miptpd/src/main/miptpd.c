#include "../lib/miptp_server.h"
#include "../lib/miptp_app_controller.h"
#include "../lib/miptp_config.h"


int main(int argc, char *argv[]){
    int rc = 0;
    
    MIPTPConfig *config = MIPTPConfig_parse_arguments(argc, argv);
    if(config == NULL) return 0;
    if(config->debug_active) printf("[MIPTP DAEMON] mode: DEBUG\tmipd-socket: %s\tapp-socket: %s\ttimeout: %ld\n", config->mipd_socket, config->app_socket, config->timeout);
    

    MIPTPServer *server = MIPTPServer_create(config->mipd_socket, config->app_socket, config->debug_active, config->timeout);
    rc = MIPTPServer_init(server);
    check(rc != -1, "Initializion of the MIPTPServer failed, aborting");
    rc = MIPTPServer_run(server);
    check(rc != -1, "MIPTP DAEMON exited unexpectedly");


    MIPTPConfig_destroy(config);
    MIPTPServer_destroy(server);
    return 0;
    
    error:
        log_err("[MIPTP DAEMON] Failed unexpectedly");
        return 1;
}