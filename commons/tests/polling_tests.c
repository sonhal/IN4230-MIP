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



char *test_func(){
    return NULL;
}


char *all_tests(){

    mu_suite_start();

    mu_run_test(test_func);

    return NULL;
}

RUN_TESTS(all_tests);