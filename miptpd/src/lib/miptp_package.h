#ifndef _MIPTP_PACKAGE_H
#define _MIPTP_PACKAGE_H

#include <stdint.h>

#include "../../../commons/src/definitions.h"

struct MIPTPHeader
{
    uint8_t PL: 2;
    uint16_t port: 14;
    uint16_t PSN;
}__attribute__((packed));

typedef struct MIPTPHeader MIPTPHeader;

typedef struct MIPTPPackage
{
    MIPTPHeader miptp_header;
    BYTE *data;
    uint16_t data_size;
} MIPTPPackage;

MIPTPPackage *MIPTPPackage_create(uint16_t port, uint16_t PSN, BYTE *data, uint16_t data_size);

void MIPTPPackage_destroy(MIPTPPackage *package);

/* Serializes the MIPTPPackage for transfer over domain socket to the MIP daemon.
    Format:
        - PL:           2 bits
        - port:         14 bit
        - PSN:          2 bytes
        - data_size:    2 bytes 
        - data:         max 1492 bits
*/
size_t MIPTPPackage_serialize(BYTE *buffer, MIPTPPackage *package);

MIPTPPackage *MIPTPPackage_deserialize(BYTE *s_package);

MIPTPPackage_serialized_get_header(BYTE *s_package, MIPTPHeader *header);

MIPTPPackage_serialized_get_data_size(BYTE *s_package, uint16_t *data_size);

MIPTPPackage_serialized_get_data(BYTE *s_package, BYTE *data);

uint8_t calc_pl(uint16_t data_size);

#endif