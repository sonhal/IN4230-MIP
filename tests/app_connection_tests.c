#include <unistd.h>		/* read, close, unlink */

#include "../src/dbg.h"
#include "minunit.h"

#include "../src/app_connection.h"

char *test_setup_app_socket(){
    return NULL;
}


char *all_tests(){

    mu_suite_start();

    mu_run_test(test_setup_app_socket);

    return NULL;
}

RUN_TESTS(all_tests);