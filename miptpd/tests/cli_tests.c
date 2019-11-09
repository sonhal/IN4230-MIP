
#include <string.h>

#include "../../commons/src/dbg.h"
#include "minunit.h"

#include "../src/lib/cli.h"


char *test_parse() {
    return NULL;
}

char *all_tests(){

    mu_suite_start();

    mu_run_test(test_parse);

    return NULL;
}

RUN_TESTS(all_tests);