#include <stdlib.h>
#include <stdint.h>

#include "dbg.h"

#include "mipd_message.h"

calc_serialized_message_size(MIPDMessage *message);

MIPDMessage *MIPDMessage_create(MIP_ADDRESS address, uint16_t data_size, BYTE *data){
    check(data_size <= MAX_MIPMESSAGE_DATA_SIZE, "data_size is to large");
    MIPDMessage *message = calloc(1, sizeof(MIPDMessage));
    check_mem(message);

    message->data = calloc(data_size, sizeof(BYTE));
    check_mem(message->data);

    message->mip_address = address;
    message->data_size = data_size;
    memcpy(message->data, data, data_size);

    return message;

    error:
        return NULL;
}


void MIPDMessage_destroy(MIPDMessage *message){
    if(message){
        if(message->data) free(message->data);
        free(message);
    }
}
/*  Creates a byte array representation of the MIPDMessage.
    Format:
        - address: 1 byte
        - data_size: 2 bytes
        - data: max 1496 bytes
    Returns the size of the size of the serialized message
*/
int MIPDMessage_serialize(BYTE *buffer, MIPDMessage *message) {
    check(buffer != NULL, "Invalid argument, buffer is NULL");
    check(message != NULL, "Invalid argument, message is NULL");
    uint16_t serialized_message_size = calc_serialized_message_size(message);

    uint16_t offset = sizeof(message->mip_address);

    memcpy(buffer, &message->mip_address, sizeof(MIP_ADDRESS));
    memcpy(&buffer[offset], &message->data_size, sizeof(message->data_size));
    offset += sizeof(message->data_size);
    memcpy(&buffer[offset], message->data, message->data_size);

    return serialized_message_size;

    error:
        log_err("Failed to serialize package in MIPDMessage_serialize");
        return NULL;
}


calc_serialized_message_size(MIPDMessage *message){
    uint16_t result = 0;
    result += sizeof(MIPDMessage) - sizeof(message->data);
    result += message->data_size;
    return result;
}

MIPDMessage *MIPDMessage_deserialize(BYTE *serialized_message){
    
    uint16_t data_size = 0;
    MIP_ADDRESS address = 0;

    MIPDMessage_serialized_get_address(serialized_message, &address);
    MIPDMessage_serialized_get_data_size(serialized_message, &data_size);
    BYTE *data = calloc(data_size, sizeof(BYTE));
    check_mem(data);

    MIPDMessage_serialized_get_data(serialized_message, data);

    MIPDMessage *message = MIPDMessage_create(address, data_size, data);

    return message;

    error:
        log_err("Failed to deserialize MIPDMessage");
        return NULL;
}



void MIPDMessage_serialized_get_address(BYTE *s_message, MIP_ADDRESS *address){
    memcpy(address, s_message, sizeof(MIP_ADDRESS));
}


void MIPDMessage_serialized_get_data_size(BYTE *s_message, uint16_t *data_size){
    memcpy(data_size, &s_message[sizeof(MIP_ADDRESS)], sizeof(uint16_t));
}

void MIPDMessage_serialized_get_data(BYTE *s_message, BYTE *data){
    uint16_t data_size = 0;
    MIPDMessage_serialized_get_data_size(s_message, &data_size);
    memcpy(data, &s_message[(sizeof(MIP_ADDRESS) + sizeof(uint16_t))], data_size);
}