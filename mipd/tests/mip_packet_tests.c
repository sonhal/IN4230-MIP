#include <unistd.h>		/* read, close, unlink */
#include <string.h>

#include "../../commons/src/dbg.h"
#include "minunit.h"

#include "../src/lib/mip_packet.h"


char *test_create_mip_packet(){
    struct ether_frame t_frame = {};
    struct mip_header m_header = {};
    char *message = "Hello";
    struct mip_packet *packet = create_mip_packet(&t_frame, &m_header, message);
    mu_assert(packet != NULL, "Packet should not be NULL");
    mu_assert(strncmp(packet->message, message, strlen(message)) == 0, "Packet message should be equal to message passed");
    return NULL;
}

char *all_tests(){

    mu_suite_start();

    mu_run_test(test_create_mip_packet);

    return NULL;
}

RUN_TESTS(all_tests);