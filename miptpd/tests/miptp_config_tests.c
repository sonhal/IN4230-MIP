
#include <string.h>

#include "../../commons/src/dbg.h"
#include "minunit.h"

#include "../src/lib/miptp_config.h"


char *test_parse() {
    char *args[] = {"miptpd", "-d", "mipdsock", "appsock", "1000"};

    MIPTPConfig *config = MIPTPConfig_parse_arguments(5, args);
    mu_assert(config != NULL, "Failed to parse args, config is NULL");
    mu_assert((strncmp(args[2] ,config->mipd_socket, strlen(args[2])) == 0), "Failed to set mipd socket correctly");
    mu_assert((strncmp(args[3] ,config->app_socket, strlen(args[3])) == 0), "Failed to set appsock socket correctly");
    mu_assert(config->debug_active == 1, "Failed to set debug correctly");
    printf("timeout: %d\n", config->timeout);
    mu_assert(config->timeout == 1000, "Failed to set timeout correctly");
    
    MIPTPConfig_destroy(config);
    return NULL;

}

char *test_parse_help() {
    char *args[] = {"miptpd", "-h"};

    MIPTPConfig *config = MIPTPConfig_parse_arguments(2, args);
    mu_assert(config == NULL, "Failed to parse args, config should be NULL when -h is present");

    
    MIPTPConfig_destroy(config);
    return NULL;

}

char *test_parse_invalid_arguments() {
    char *args[] = {"miptpd", "-s", "euq"};

    MIPTPConfig *config = MIPTPConfig_parse_arguments(2, args);
    mu_assert(config == NULL, "Failed to parse args, config should be NULL when invalid arguments are present");

    
    MIPTPConfig_destroy(config);
    return NULL;

}



char *all_tests(){

    mu_suite_start();

    mu_run_test(test_parse_invalid_arguments);
    mu_run_test(test_parse);
    mu_run_test(test_parse_help);
    

    return NULL;
}

RUN_TESTS(all_tests);