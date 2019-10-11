#include <stdio.h>

#include "../../../commons/src/dbg.h"

#include "../lib/cli.h"
#include "../lib/router_server.h"

void clean_up(RouterdConfig *config, RouterServer *server){
    if(config) RouterdConfig_destroy(config);
    if(server) RouterServer_destroy(server);
}

int main(int argc, char *argv[]){
    printf("Routing up!\n");
    int rc = 0;
    RouterdConfig *config = NULL;
    RouterServer *server = NULL;

    config = parse_args(argc, argv);
    if(config == NULL) return 0;

    server = RouterServer_create(config);
    check_mem(server);

    rc = RouterServer_init(server);
    check(rc != -1, "Failed to initialize server");
    rc = RouterServer_run(server);
    check(rc != -1, "");

    clean_up(config, server);
    return 0;

    error:
        clean_up(config, server);
        printf("[ROUTERD] Failed unexpectedly.\n");
        return -1;

}