#ifndef _MIPTP_PACKAGE_H
#define _MIPTP_PACKAGE_H

#include <stdint.h>
#include <stdlib.h>

#include "../../../commons/src/definitions.h"

#define MAX_DATA_SIZE_BYTES 1492

typedef struct ClientPackage {
    uint16_t port;
    MIP_ADDRESS destination;
    uint16_t data_size;
    BYTE *data;
} ClientPackage;



/*  Creates a byte array representation of the ClientPackage.
    Format:
        - port: 2 bytes
        - destination: 1 byte
        - data_size: 2 bytes
        - data: max 65535 bytes */
BYTE *ClientPackage_serialize(ClientPackage *package);

ClientPackage *ClientPackage_deserialize(BYTE *serialized_package);

void ClientPackage_serialized_get_port(BYTE *s_package, uint16_t *port);

void ClientPackage_serialized_get_destination(BYTE *s_package, MIP_ADDRESS *destination);

void ClientPackage_serialized_get_data_size(BYTE *s_package, uint16_t *data_size);

void ClientPackage_serialized_get_data(BYTE *s_package, BYTE *data);

ClientPackage *ClientPackage_create(uint16_t port, MIP_ADDRESS destination, BYTE *data, uint16_t data_size);

void ClientPackage_destroy(ClientPackage *package);

#endif