#include "../../commons/src/dbg.h"
#include "minunit.h"

#include "../src/bound_socket.h"



char *test_BoundSocket_create_destroy(){

    BoundSocket *socket = BoundSocket_create("testsock");
    mu_assert(socket != NULL, "failed to create BoundSocket");
    BoundSocket_destroy(socket);

    return NULL;
}

char *test_BoundSocket_connect(){

    int rc = 0; 
    BoundSocket *socket = BoundSocket_create("testsock");
    mu_assert(socket != NULL, "failed to create BoundSocket");
    rc = BoundSocket_connect(socket);
    mu_assert(rc != -1, "Failed to connect BoundSocket");
    BoundSocket_destroy(socket);

    return NULL;
}


char *all_tests(){

    mu_suite_start();

    mu_run_test(test_BoundSocket_create_destroy);
    mu_run_test(test_BoundSocket_connect);
    return NULL;
}

RUN_TESTS(all_tests);