#include "minunit.h"
#include "../src/queue.h"
#include <assert.h>


static Queue *queue = NULL;

char *tests[] = { "test1 data", "test2 data", "test3 data" };

#define NUM_TESTS 3


char *test_create(){
    queue = Queue_create();
    mu_assert(queue != NULL, "Failed to create queue.");

    return NULL;
}

char *test_destroy() {
    mu_assert(queue != NULL, "Failed to make queue #2");
    Queue_destroy(queue);

    return NULL;
}

char *test_foreach(){

    queue = Queue_create();
    Queue_send(queue, tests[0]);
    Queue_send(queue, tests[1]);
    Queue_send(queue, tests[2]);

    QUEUE_FOREACH(queue, cur){
        debug("foreach value: %s", (char *)Queue_recv(queue));
    }
    debug("done");

    Queue_destroy(queue);
    return NULL;
}

char *test_send_recv(){

    queue = Queue_create();

    int i = 0;
    for(i = 0; i < NUM_TESTS; i++){
        Queue_send(queue, tests[i]);
        debug("value: %s", (char *)Queue_peek(queue));
        mu_assert(Queue_peek(queue) == tests[0], "Wrong  next value");
    }

    mu_assert(Queue_count(queue) == NUM_TESTS, "Wrong count of values.");

    QUEUE_FOREACH(queue, cur){
        debug("VAL: %s", (char *)cur->value);
    }

    for(i = 0; i < NUM_TESTS; i++){
        char *val = Queue_recv(queue);
        mu_assert(val == tests[i], "Wrong value on recv.");
    }

    mu_assert(Queue_count(queue) == 0, "Wrong count after recv.");

    return NULL;
}

char *all_tests(){

    mu_suite_start();

    mu_run_test(test_create);
    mu_run_test(test_destroy);
    mu_run_test(test_foreach);
    mu_run_test(test_send_recv);

    return NULL;
}

RUN_TESTS(all_tests);