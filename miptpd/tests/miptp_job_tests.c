#include <string.h>

#include "../../commons/src/dbg.h"
#include "minunit.h"

#include "../src/lib/miptp_job.h"


char *test_MIPTPJob_create_destroy() {
    MIPTPJob *job = MIPTPJob_create("Hello", strlen("Hello"), 1000);
    mu_assert(job != NULL, "Failed to create MIPTPAppController");
    mu_assert(job->sliding_window.sequence_base == 0, "SlidingWindow base is wrong");
    mu_assert(job->sliding_window.sequence_max == WINDOW_SIZE + 1, "SlidingWindow max is wrong");
    mu_assert(job->sliding_window.window_size == WINDOW_SIZE, "SlidingWindow window_size is wrong");
    mu_assert(job->timeout_len == 1000, "Timeout is wrong");
    mu_assert(strncmp(job->data, "Hello", strlen("Hello")) == 0, "data is wrong");

    MIPTPJob_destroy(job);
    
    return NULL;
}


char *all_tests(){

    mu_suite_start();
    mu_run_test(test_MIPTPJob_create_destroy);

    return NULL;
}

RUN_TESTS(all_tests);