#include <unistd.h>		/* read, close, unlink */
#include <sys/socket.h>
#include <unistd.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <stdint.h>
#include <stdlib.h> 
#include <netinet/in.h> 

#include "../../commons/src/dbg.h"
#include "minunit.h"
#include "../src/lib/packaging/ether_frame.h"
#include "../src/lib/interface.h"


char *test_ether_frame_to_string() {
    struct sockaddr_ll *so_names = calloc(10, SOCKET_ADDR_SIZE);
    int interface_n = collect_intefaces(so_names, 10);
    uint8_t dest_e_a[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    struct ether_frame *e_frame = create_ethernet_frame(dest_e_a, &so_names[0]);
    char *e_str = ether_frame_to_string(e_frame);
    printf("%s", e_str);

    return NULL;
}


char *all_tests(){

    mu_suite_start();

    mu_run_test(test_ether_frame_to_string);

    return NULL;
}

RUN_TESTS(all_tests);