#include <stdint.h>
#include "../lib/packaging/mip_header.h"

typedef struct UserConfig {
    int is_debug;
    char *app_socket;
    char *route_socket;
    char *forward_socket;
    MIP_ADDRESS *mip_addresses;
    size_t num_mip_addresses;
} UserConfig;


UserConfig *UserConfig_create(size_t n_interfaces);

void UserConfig_destroy(UserConfig*config);

UserConfig *UserConfig_from_cli(int argc, char *argv[], size_t n_interfaces);