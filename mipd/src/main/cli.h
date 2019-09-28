#include <stdint.h>
#include "../lib/mip.h"

struct user_config {
    int is_debug;
    char *app_socket;
    MIP_ADDRESS *mip_addresses;
    int64_t num_mip_addresses;
};


struct user_config *create_user_config(uint64_t n_interfaces);

int destroy_user_config(struct user_config *config);

struct user_config *handle_user_config(int argc, char *argv[], uint64_t n_interfaces);