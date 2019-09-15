#include <unistd.h>		/* read, close, unlink */

#include "../src/dbg.h"
#include "minunit.h"

#include "../src/link.c"

char *test_print_interface_list(){
    print_interface_list();
    return NULL;
}


char *all_tests(){

    mu_suite_start();

    mu_run_test(test_print_interface_list);

    return NULL;
}

RUN_TESTS(all_tests);