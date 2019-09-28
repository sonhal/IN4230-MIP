#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/epoll.h> 
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>

#include "../../commons/src/dbg.h"
#include "minunit.h"

#include "../src/polling.h"



void test_func(int i){
    printf("Hello: %d\n", i);
}

char *test_create_event_handler(){
    struct event_handler new_handler = create_event_handler(1, &test_func);
    (*new_handler.handler_func)(10);
    return NULL;
}


char *all_tests(){

    mu_suite_start();

    mu_run_test(test_create_event_handler);

    return NULL;
}

RUN_TESTS(all_tests);