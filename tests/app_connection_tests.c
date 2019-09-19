#include <unistd.h>		/* read, close, unlink */

#include "../src/dbg.h"
#include "minunit.h"

#include "../src/app_connection.c"

char *test_create_domain_socket(){
    int so = 1;
    struct sockaddr_un so_name;
    so = create_domain_socket(&so_name, "appsocket", sizeof("appsocket"));
    close(so);
    unlink("appsocket");
    mu_assert(so != -1, "Should be valid socket");
    return NULL;
}


char *all_tests(){

    mu_suite_start();

    //mu_run_test(test_create_domain_socket);

    return NULL;
}

RUN_TESTS(all_tests);