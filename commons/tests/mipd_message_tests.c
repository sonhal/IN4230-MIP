#include <string.h>

#include "../src/dbg.h"
#include "minunit.h"

#include "../src/mipd_message.h"


char *test_MIPDMessage_serialize() {
    MIPDMessage *message = MIPDMessage_create(110, strlen("Hello"), "Hello");
    mu_assert(message != NULL, "Failed to create MIPTmessage");
    
    int s_size = 0;
    BYTE *serialized_message = calloc(100, sizeof(char));
    s_size = MIPDMessage_serialize(serialized_message, message);
    mu_assert(serialized_message != NULL, "Failed to serialize message");
    mu_assert(s_size != 0, "Failed to serialize message, size is 0");
    mu_assert(s_size == (sizeof(MIP_ADDRESS) + sizeof(uint16_t) + strlen("Hello")), "Wrong serialize size");
    char *ser_string = calloc(MAX_MIPMESSAGE_DATA_SIZE, sizeof(BYTE));

    MIPDMessage_serialized_get_data(serialized_message, ser_string);
    mu_assert(strncmp(message->data, ser_string, message->data_size) == 0, "Serialized data does not match original data");

    MIP_ADDRESS destination = 0;
    MIPDMessage_serialized_get_address(serialized_message, &destination);
    mu_assert(destination == 110, "Failed to get destination from serialized message");



    MIPDMessage_destroy(message);
    free(serialized_message);
    return NULL;
}

char *test_MIPDMessage_deserialize() {
    MIPDMessage *message = MIPDMessage_create(11, strlen("Hello"), "Hello");
    mu_assert(message != NULL, "Failed to create MIPTmessage");
    
    int s_size = 0;
    BYTE *serialized_message = calloc(100, sizeof(char));
    s_size = MIPDMessage_serialize(serialized_message, message);
    mu_assert(serialized_message != NULL, "Failed to serialize message");
    MIPDMessage *message2 = MIPDMessage_deserialize(serialized_message);

    mu_assert(message2 != NULL, "Failed to deserialize message, pointer is NULL");
    mu_assert(message->mip_address == message2->mip_address, "Bad deserialization, address is not equal");
    mu_assert(message->data_size == message2->data_size, "Bad deserialization, data_size is not equal");
    mu_assert(strncmp(message->data, message2->data, message->data_size) == 0, "Bad deserialization, data is not equal");


    MIPDMessage_destroy(message);
    MIPDMessage_destroy(message2);
    free(serialized_message);
    return NULL;
}

char *test_MIPDMessage_deserialize_no_data_message() {
    MIPDMessage *message = MIPDMessage_create(11, 0, NULL);
    mu_assert(message != NULL, "Failed to create MIPTmessage");
    
    int s_size = 0;
    BYTE *serialized_message = calloc(100, sizeof(char));
    s_size = MIPDMessage_serialize(serialized_message, message);
    mu_assert(serialized_message != NULL, "Failed to serialize message");
    MIPDMessage *message2 = MIPDMessage_deserialize(serialized_message);

    mu_assert(message2 != NULL, "Failed to deserialize message, pointer is NULL");
    mu_assert(message->mip_address == message2->mip_address, "Bad deserialization, address is not equal");
    mu_assert(message->data_size == message2->data_size, "Bad deserialization, data_size is not equal");
    mu_assert(strncmp(message->data, message2->data, message->data_size) == 0, "Bad deserialization, data is not equal");


    MIPDMessage_destroy(message);
    MIPDMessage_destroy(message2);
    free(serialized_message);
    return NULL;
}

char *test_MIPDMessage_create_destroy() {
    MIPDMessage *message = MIPDMessage_create(110, strlen("Hello"), "Hello");
    mu_assert(message != NULL, "Failed to create MIPTmessage");
    mu_assert(message->data_size == strlen("Hello"), "Failed to set data_size");
    mu_assert(message->mip_address == 110, "Failed to set destination");
    mu_assert(strncmp("Hello", message->data, strlen("Hello")) == 0, "Failed to set data");

    MIPDMessage_destroy(message);
    message = NULL;
    MIPDMessage_destroy(message);

    return NULL;
}

char *all_tests(){

    mu_suite_start();

    mu_run_test(test_MIPDMessage_create_destroy);
    mu_run_test(test_MIPDMessage_serialize);
    mu_run_test(test_MIPDMessage_deserialize);
    mu_run_test(test_MIPDMessage_deserialize_no_data_message);

    return NULL;
}

RUN_TESTS(all_tests);