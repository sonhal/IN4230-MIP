#include <string.h>

#include "../../commons/src/dbg.h"
#include "minunit.h"

#include "../src/lib/miptp_server.h"


char *test_MIPTPServer_create_destroy() {
    MIPTPServer *server = MIPTPServer_create("mipdsock", "appsock", 1, 1000);
    mu_assert(server != NULL, "Failed to create server");
    mu_assert(MIPTPServer_is_debug_active(server), "Debug should be active");
    mu_assert(strncmp("mipdsock", server->mipd_socket, strlen("mipdsock")) == 0, "Failed to set mipd socket correctly");
    mu_assert(strncmp("appsock", server->app_connections_socket->file_path, strlen("appsock")) == 0, "Failed to set app socket correctly");

    MIPTPServer_destroy(server);

    return NULL;
}

char *test_MIPTPServer_init() {
    MIPTPServer *server = MIPTPServer_create("mipdsock", "appsock", 1, 1000);
    mu_assert(server != NULL, "Failed to create server");
    
    int rc = MIPTPServer_init(server);
    mu_assert(rc != -1, "Failed to init MIPTPServer");

    MIPTPServer_destroy(server);

    return NULL;
}



char *all_tests(){

    mu_suite_start();

    mu_run_test(test_MIPTPServer_create_destroy);
    mu_run_test(test_MIPTPServer_init);

    return NULL;
}

RUN_TESTS(all_tests);