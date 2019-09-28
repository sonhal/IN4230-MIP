#include <unistd.h>		/* read, close, unlink */
#include <sys/socket.h>
#include <unistd.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <stdint.h>
#include <stdlib.h> 
#include <netinet/in.h> 
#include <sys/socket.h>
#include <sys/un.h>


#include "../../commons/src/dbg.h"
#include "minunit.h"

#include "../src/lib/app_connection.h"

char *test_create_domain_socket(){
    int so = 1;
    struct sockaddr_un so_name;
    so = create_domain_socket(&so_name, "appsocket", sizeof("appsocket"));
    close(so);
    unlink("appsocket");
    mu_assert(so != -1, "Should be valid socket");
    return NULL;
}


char *test_parse_domain_socket_input(){
    char *buffer = "12 ping";
    int buff_n = strlen(buffer);
    uint8_t mip_addr;
    char *message = NULL;
    parse_domain_socket_request(buffer, buff_n, &mip_addr);
    printf("mip address: %d", mip_addr);
    return NULL;
}



char *all_tests(){

    mu_suite_start();

    //mu_run_test(test_create_domain_socket);
    mu_run_test(test_parse_domain_socket_input);

    return NULL;
}

RUN_TESTS(all_tests);