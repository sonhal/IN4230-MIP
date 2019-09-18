#include <unistd.h>		/* read, close, unlink */

#include "../src/dbg.h"
#include "minunit.h"

#include "../src/link.c"

char *test_print_interface_list(){
    struct sockaddr_ll *so_name = calloc(1, sizeof(struct sockaddr_ll));
    last_inteface(so_name);
    free(so_name);
    return NULL;
}


char *all_tests(){

    mu_suite_start();

    mu_run_test(test_print_interface_list);

    return NULL;
}

RUN_TESTS(all_tests);