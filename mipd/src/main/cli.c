#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../../../commons/src/dbg.h"
#include "cli.h"

UserConfig *UserConfig_create(uint64_t mip_address_space_size){
    UserConfig *new_config = calloc(1, sizeof(UserConfig));
    new_config->mip_addresses = calloc(mip_address_space_size, sizeof(MIP_ADDRESS));
    return new_config;
}

void UserConfig_destroy(UserConfig *config){
    if(config->app_socket)free(config->app_socket);
    free(config);
}

int fetch_mip_addresses(int argc, char *argv[], int offset, uint8_t *mip_addresses[], int address_n){
    check((argc - offset - 1) <= address_n, "To many mip addresses provided - max: %d", address_n);
    check((argc - offset) > 0, "To few mip addresses provided - min: 1, %d where provided", (argc - offset));

    int  i = 0;
    int k = 0;
    for ( i = offset +1; i < argc; i++){
        uint8_t new_mip_addr = atoi(argv[i]);
        check(new_mip_addr < 256, "MIP address value provided is to large, max: 255");
        debug("mip index %d, set to %d", k, new_mip_addr);
        mip_addresses[k] = new_mip_addr;
        k++;
    }
    return k;

    error:
        return -1;
}

UserConfig *UserConfig_from_cli(int argc, char *argv[], size_t n_interfaces){
    int rc = 0;
    UserConfig *config = UserConfig_create(n_interfaces);
    int offset = 1;
    if (!strncmp(argv[1], "-d", 2)){
        printf("[SERVER] mipd started in debug mode\n");
        config->is_debug = 1;
        offset++;
    }
    // Parse app socket
    config->app_socket = calloc(1, strlen(argv[offset]) + 1);        
    strncpy(config->app_socket, argv[offset], strlen(argv[offset]));
    offset++;
    // Parse route socket
    config->route_socket = calloc(1, strlen(argv[offset]) + 1);        
    strncpy(config->route_socket, argv[offset], strlen(argv[offset]));
    offset++;
    // Parse forward socket
    config->forward_socket = calloc(1, strlen(argv[offset]) + 1);        
    strncpy(config->forward_socket, argv[offset], strlen(argv[offset]));

    rc = fetch_mip_addresses(argc, argv, offset, config->mip_addresses, n_interfaces);
    check(rc != -1, "Failed to fetch mip addresses");
    config->num_mip_addresses = rc;

    return config;

    error:
        UserConfig_destroy(config);
        return NULL;
}