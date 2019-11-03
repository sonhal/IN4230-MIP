#include <stdlib.h>
#include <string.h>

#include "../../../commons/src/dbg.h"

#include "miptp_package.h"


MIPTPPackage *MIPTPPackage_create(uint8_t PL, uint16_t port, uint16_t PSN, BYTE *data, uint16_t data_size){
    MIPTPPackage *package = calloc(1, sizeof(MIPTPPackage));
    package->miptp_header.PL = PL;
    package->miptp_header.port = port;
    package->miptp_header.PSN = PSN;

    package->data = calloc(data_size, sizeof(BYTE));
    check_mem(data);
    package->data_size = data_size;
    memcpy(package->data, data, package->data_size);

    return package;

    error:
        log_err("Failed to create MIPTPPackage in MIPTPPackage_create");
        return NULL;
}

void MIPTPPackage_destroy(MIPTPPackage *package){
    if(package){
        if(package->data){
            free(package->data);
        }
        free(package);
    }
}

BYTE *MIPTPPackage_serialize(MIPTPPackage *package){
    // Size of MIPTPPackage minus the data pointer
    size_t package_size = sizeof(MIPTPPackage) - sizeof(BYTE *);
    package_size += package->data_size;
    BYTE *s_package = calloc(package_size, sizeof(BYTE));

    memcpy(s_package, &package->miptp_header, sizeof(MIPTPHeader));
    memcpy(&s_package[sizeof(MIPTPHeader)], &package->data_size, sizeof(uint16_t));
    memcpy(&s_package[sizeof(MIPTPHeader) + sizeof(uint16_t)], package->data, package->data_size);

    return s_package;

    error:
        log_err("Failed to serialize MIPTPPackage in MIPTPPackage_serialize");
        return NULL;
}

MIPTPPackage *MIPTPPackage_deserialize(BYTE *s_package){

    uint16_t data_size = 0;
    MIPTPHeader header = {};

    MIPTPPackage_serialized_get_data_size(s_package, &data_size);
    BYTE *data = calloc(data_size, sizeof(BYTE));
    check_mem(data);

    MIPTPPackage_serialized_get_data(s_package, data);
    MIPTPPackage_serialized_get_header(s_package, &header);

    MIPTPPackage *package = MIPTPPackage_create(header.PL, header.port, header.PSN, data, data_size);
    check(package != NULL, "Failed to create MIPTPPackage for deserialized data");

    return package;

    error:
        log_err("Failed to deserialize MIPTPPackage in MIPTPPackage_deserialize");
        return NULL;
}

MIPTPPackage_serialized_get_header(BYTE *s_package, MIPTPHeader *header){
    memcpy(header, s_package, sizeof(MIPTPHeader));
}
MIPTPPackage_serialized_get_data_size(BYTE *s_package, uint16_t *data_size){
    memcpy(data_size, &s_package[sizeof(MIPTPHeader)], sizeof(uint16_t));
}
MIPTPPackage_serialized_get_data(BYTE *s_package, BYTE *data){
    uint16_t data_size = 0;
    MIPTPPackage_serialized_get_data_size(s_package, &data_size);

    memcpy(data, &s_package[sizeof(MIPTPHeader) + sizeof(uint16_t)], data_size);
}
