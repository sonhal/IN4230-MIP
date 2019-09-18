#include <unistd.h>		/* read, close, unlink */

#include "../src/dbg.h"
#include "minunit.h"

#include "../src/mip.c"



char *test_create_mip_arp_packet(){
    struct mip_header *header;
    header = create_arp_request_package(64);
    mu_assert(header->src_addr == 64, "Wrong value");
    mu_assert(header->dst_addr == 255, "Wrong value");
    mu_assert(header->tra == 1, "Wrong value");

    free(header);
    return NULL;
}

char *test_create_mip_arp_response_packet(){
    struct mip_header *request;
    struct mip_header *response;
    int8_t response_mip_addr = 128;

    request = create_arp_request_package(64);
    response = create_arp_response_package(response_mip_addr, request);
    mu_assert(response->src_addr == 128, "Wrong src value");
    mu_assert(response->dst_addr == request->src_addr, "Wrong dest value");
    mu_assert(response->tra == 1, "Wrong value");

    free(request);
    free(response);
    return NULL;
}

char *all_tests(){

    mu_suite_start();

    mu_run_test(test_create_mip_arp_packet);
    mu_run_test(test_create_mip_arp_response_packet);

    return NULL;
}

RUN_TESTS(all_tests);