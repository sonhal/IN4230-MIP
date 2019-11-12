#ifndef _MIPD_MESSAGE_H
#define _MIPD_MESSAGE_H

#include <stdint.h>

#include "definitions.h"

#define MAX_MIPMESSAGE_DATA_SIZE 1496
#define MAX_MIPMESSAGE_SIZE 1500

typedef struct MIPDMessage {
    MIP_ADDRESS mip_address;
    uint16_t data_size;
    BYTE *data;
} MIPDMessage;


MIPDMessage *MIPDMessage_create(MIP_ADDRESS address, uint16_t data_size, BYTE *data);

void MIPDMessage_destroy(MIPDMessage *message);

/*  Creates a byte array representation of the MIPDMessage.
    Format:
        - address: 1 byte
        - data_size: 2 bytes
        - data: max 1496 bytes
    Returns the size of the size of the serialized message
*/
int MIPDMessage_serialize(BYTE *buffer, MIPDMessage *message);

MIPDMessage *MIPDMessage_deserialize(BYTE *serialized_message);

void MIPDMessage_serialized_get_address(BYTE *s_message, MIP_ADDRESS *address);

void MIPDMessage_serialized_get_data_size(BYTE *s_message, uint16_t *data_size);

void MIPDMessage_serialized_get_data(BYTE *s_message, BYTE *data);

#endif