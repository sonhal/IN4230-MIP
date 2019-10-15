#ifndef _ROUTERD_CLI_H
#define _ROUTERD_CLI_H

#include "../../../commons/src/list.h"

#define BROADCAST_FREQ_MILLI 10000

typedef struct RouterdConfig {
    char *routing_socket;
    char *forwarding_socket;
    List *mip_addresses;
    int debug_active;
    long broadcast_freq;
} RouterdConfig;


// Parses CLI arguments and returns the user configurations as a struct pointer, returns NULL on failure
RouterdConfig *parse_args(int argc, char *argv[]);


RouterdConfig *RouterdConfig_create(int argc, char *argv[]);

void RouterdConfig_destroy(RouterdConfig *config);

#define RouterdConfig_is_debug_active(C) ((C)->debug_active ? 1 : 0)

#endif