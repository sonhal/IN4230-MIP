#include "dbg.h"

#include "client_package.h"

uint16_t calc_serialized_package_size(ClientPackage *package);


/*  Creates a byte array representation of the ClientPackage.
    Format:
        - port: 2 bytes
        - destination: 1 byte
        - type: 4 bytes
        - data_size: 2 bytes
        - data: max 65535 bytes
    Returns the size of the size of the serialized package
*/
int ClientPackage_serialize(BYTE *buffer, ClientPackage *package) {
    uint16_t serialized_package_size = calc_serialized_package_size(package);

    uint16_t offset = sizeof(package->port);

    memcpy(buffer, &package->port, sizeof(package->port));
    memcpy(&buffer[offset], &package->destination, sizeof(package->destination));
    offset += sizeof(package->destination);
    memcpy(&buffer[offset], &package->type, sizeof(enum MIPTPClientType));
    offset += sizeof(enum MIPTPClientType);
    memcpy(&buffer[offset], &package->data_size, sizeof(package->data_size));
    offset += sizeof(package->data_size);
    memcpy(&buffer[offset], package->data, package->data_size);

    return serialized_package_size;

    error:
        log_err("Failed to serialize package in ClientPackage_serialize");
        return NULL;
}

ClientPackage *ClientPackage_deserialize(BYTE *serialized_package){
    
    uint16_t data_size = 0;
    uint16_t port = 0;
    MIP_ADDRESS destination = 0;
    enum MIPTPClientType type = 0;
    ClientPackage_serialized_get_data_size(serialized_package, &data_size);
    BYTE *data = calloc(data_size, sizeof(BYTE));
    check_mem(data);

    ClientPackage_serialized_get_data(serialized_package, data);
    ClientPackage_serialized_get_port(serialized_package, &port);
    ClientPackage_serialized_get_destination(serialized_package, &destination);
    ClientPackage_serialized_get_type(serialized_package, &type);

    ClientPackage *package = ClientPackage_create(port, destination, type, data, data_size);

    return package;

    error:
        log_err("Failed to deserialize ClientPackage");
        return NULL;
}

uint16_t calc_serialized_package_size(ClientPackage *package){
    uint16_t result = 0;
    result += sizeof(ClientPackage) - sizeof(package->data);
    result += package->data_size;
    return result;
}


ClientPackage *ClientPackage_create(uint16_t port, MIP_ADDRESS destination, enum MIPTPClientType type, BYTE *data, uint16_t data_size){

    if(data_size > MAX_DATA_SIZE_BYTES){
        log_err("data size is to large: data_size=%zu", data_size);
        goto error;
    }

    ClientPackage *package = calloc(1, sizeof(ClientPackage));
    check_mem(package);

    package->port = port;
    package->destination = destination;
    package->data_size = data_size;
    package->type = type;

    package->data = calloc(data_size, sizeof(BYTE));
    check_mem(package->data);
    memcpy(package->data, data, data_size);

    return package;

    error:
        log_err("Failed to create ClientPackage in ClientPackage_create");
        return NULL;
}

void ClientPackage_destroy(ClientPackage *package){
    if(package){
        if(package->data){
            free(package->data);
        }
        free(package);
        package = NULL;
    }
}

void ClientPackage_serialized_get_port(BYTE *s_package, uint16_t *port){
    memcpy(port, s_package, sizeof(uint16_t));
}

void ClientPackage_serialized_get_destination(BYTE *s_package, MIP_ADDRESS *destination){
    memcpy(destination, &s_package[sizeof(uint16_t)], sizeof(MIP_ADDRESS));
}

void ClientPackage_serialized_get_type(BYTE *s_package, enum MIPTPClientType *type){
    memcpy(type, &s_package[(sizeof(uint16_t) + sizeof(MIP_ADDRESS))], sizeof(enum MIPTPClientType));
}

void ClientPackage_serialized_get_data_size(BYTE *s_package, uint16_t *data_size){
    memcpy(data_size, &s_package[(sizeof(uint16_t) + sizeof(MIP_ADDRESS) + sizeof(enum MIPTPClientType))], sizeof(uint16_t));
}

void ClientPackage_serialized_get_data(BYTE *s_package, BYTE *data){
    uint16_t data_size = 0;
    ClientPackage_serialized_get_data_size(s_package, &data_size);
    memcpy(data, &s_package[(sizeof(uint16_t) * 2) + sizeof(MIP_ADDRESS) + sizeof(enum MIPTPClientType)], data_size);
}