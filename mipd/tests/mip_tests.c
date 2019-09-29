#include <unistd.h>		/* read, close, unlink */

#include "../../commons/src/dbg.h"
#include "minunit.h"

#include "../src/lib/mip_header.h"


char *test_create_mip_arp_packet(){
    struct mip_header *header;
    header = create_arp_request_package(64);
    mu_assert(header->src_addr == 64, "Wrong src value");
    mu_assert(header->dst_addr == 255, "Wrong dst value");
    mu_assert(header->tra == 1, "Wrong tra value");

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
    mu_assert(response->tra == 0, "Wrong response tra value");

    free(request);
    free(response);
    return NULL;
}

char *test_mip_header_to_string(){
    struct mip_header *request;
    request = create_arp_request_package(64);

    char *mip_h_str = mip_header_to_string(request);
    printf("%s", mip_h_str);
    free(request);
    free(mip_h_str);
    return NULL;
}

char *all_tests(){

    mu_suite_start();

    mu_run_test(test_create_mip_arp_packet);
    mu_run_test(test_create_mip_arp_response_packet);
    mu_run_test(test_mip_header_to_string);

    return NULL;
}

RUN_TESTS(all_tests);