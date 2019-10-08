#include <inttypes.h>

#include "../../../commons/src/dbg.h"
#include "../../../commons/src/definitions.h"

#include "cli.h"


// Parses CLI arguments and returns the user configurations as a struct pointer
RouterdConfig *parse_args(int argc, char *argv[]){
    check(argc > 3, "To few arguments, argc: %d", argc);
    check(argc < 258, "To many arguments, argc: %d", argc);

    return RouterdConfig_create(argc, argv);

    error:
        printf("routerd [-h] [-d] <routing_socket> <forwarding_socket> [MIP addresses ...]");
        return NULL;
}


// Creates a RouterConfig struct, does not validate args as it should be done in parse_args()
RouterdConfig *RouterdConfig_create(int argc, char *argv[]){
    int  i = 1;
    int rc = 0;
    int forward_socket_offset = -1;
    RouterdConfig * config = calloc(1, sizeof(RouterdConfig));
    config->broadcast_freq = BROADCAST_FREQ_MILLI;
    config->mip_addresses = List_create();

    for(i = 1; i < argc; i++){
        if(i == 1){
            if(!(strncmp("-h", argv[i], 2))){
                printf("routerd [-h] [-d] <routing_socket> <forwarding_socket> [MIP addresses ...]");
                return NULL;
            }
            if(!(strncmp("-d", argv[i], 2))){
                config->debug_active = 1;
                i++;
            }
            config->routing_socket = calloc(1, strlen(argv[i]));
            memcpy(config->routing_socket, argv[i], strlen(argv[i]));
            forward_socket_offset = i + 1;
        } else if(i == forward_socket_offset){
            config->forwarding_socket = calloc(1, strlen(argv[i]) + 1);
            memcpy(config->forwarding_socket, argv[i], strlen(argv[i]) + 1);
        } else {
            MIP_ADDRESS *address = calloc(1, sizeof(MIP_ADDRESS));
            rc = sscanf(argv[i], "%"SCNu8, address);
            List_push(config->mip_addresses, address);
        }

    }

    return config;

    error:
        return NULL;
}

void RouterdConfig_destroy(RouterdConfig *config){
    if(config){
        if(config->forwarding_socket) free(config->forwarding_socket);
        if(config->routing_socket) free(config->routing_socket);
        if(config->mip_addresses) List_clear_destroy(config->mip_addresses);
        free(config);
    }
}