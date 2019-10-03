#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../../../commons/src/dbg.h"
#include "cli.h"

struct user_config *create_user_config(uint64_t mip_address_space_size){
    struct user_config *new_config = calloc(1, sizeof(struct user_config));
    new_config->mip_addresses = calloc(mip_address_space_size, sizeof(MIP_ADDRESS));
    return new_config;
}

int destroy_user_config(struct user_config *config){
    if(config->app_socket)free(config->app_socket);
    if(config->num_mip_addresses)free(config->num_mip_addresses);
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

struct user_config *handle_user_config(int argc, char *argv[], uint64_t n_interfaces){
    int rc = 0;
    struct user_config *config = create_user_config(n_interfaces);
    int offset = 1;
    if (!strncmp(argv[1], "-d", 2)){
        printf("[SERVER] mipd started in debug mode\n");
        config->is_debug = 1;
        offset++;
    }
    config->app_socket = calloc(1, strlen(argv[offset]) * sizeof(char) + 1);        
    strncpy(config->app_socket, argv[offset], strlen(argv[offset]));
    rc = fetch_mip_addresses(argc, argv, offset, config->mip_addresses, n_interfaces);
    check(rc != -1, "Failed to fetch mip addresses");
    config->num_mip_addresses = rc;

    return config;

    error:
        destroy_user_config(config);
        return NULL;
}