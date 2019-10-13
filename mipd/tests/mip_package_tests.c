#include <unistd.h>		/* read, close, unlink */
#include <string.h>

#include "../../commons/src/dbg.h"
#include "minunit.h"

#include "../src/lib/packaging/mip_package.h"


char *test_create_mip_packet(){
    struct ether_frame t_frame = {};
    struct mip_header m_header = {};
    const BYTE *message = "Hello";
    MIPPackage *packet = MIPPackage_create(&t_frame, &m_header, message, strlen(message));
    mu_assert(packet != NULL, "Packet should not be NULL");
    mu_assert(strncmp((char *)packet->message, (char *)message, strlen((char *)message)) == 0, "Packet message should be equal to message passed");
    return NULL;
}

char *test_mip_packet_to_string(){
    struct ether_frame t_frame = {};
    struct mip_header m_header = {};
    BYTE *message = "Hello";
    MIPPackage *packet = MIPPackage_create(&t_frame, &m_header, message, strlen("Hello"));
    char *m_p_str = MIPPackage_to_string(packet);
    printf("%s", m_p_str);
    free(m_p_str);
    return NULL;
}


char *test_calculate_payload() {
    char *payload = "MY PAYLOAD";
    size_t payload_size = strlen(payload);
    printf("payload size: %lu\n", payload_size);
    int payload_words = calculate_mip_payload_words(payload_size);
    int payload_padding = (payload_words * 4) - payload_size;
    printf("number of words: %d\n", payload_words);
    printf("bytes of paddding: %d\n", payload_padding);
    mu_assert(payload_words == 3, "Should only need one word");
    mu_assert(payload_padding == 2, "Should be 2 bytes of padding");
    return NULL;
}

char *all_tests(){

    mu_suite_start();

    mu_run_test(test_create_mip_packet);
    mu_run_test(test_mip_packet_to_string);
    mu_run_test(test_calculate_payload);

    return NULL;
}

RUN_TESTS(all_tests);