#include "../../commons/src/dbg.h"
#include "minunit.h"
#include "../src/lib/cli.h"
#include <string.h>


char *test_RouterdConfig_create() {
    int argc = 5;
    char *argv[] = {"routerd", "route_socket", "forward_socket", "10", "20"};
    int result = 0;

    RouterdConfig *config = RouterdConfig_create(argc, argv);
    mu_assert(config != NULL, "RouterdConfig is NULL");
    result = strncmp(argv[1], config->routing_socket, strlen(argv[1]));
    mu_assert(result == 0, "Wrong route socket parsed");

    RouterdConfig_destroy(config);
    return NULL;
}


char *test_RouterdConfig_debug_active() {
    int argc = 6;
    char *argv[] = {"routerd", "-d" "route_socket", "forward_socket", "10", "20"};

    RouterdConfig *config = RouterdConfig_create(argc, argv);
    mu_assert(RouterdConfig_is_debug_active(config), "Debug should be active");

    int argc2 = 5;
    char *argv2[] = {"routerd", "route_socket", "forward_socket", "10", "20"};

    RouterdConfig *config2 = RouterdConfig_create(argc2, argv2);
    mu_assert(RouterdConfig_is_debug_active(config2) == 0, "Debug should not be active");

    RouterdConfig_destroy(config);
    RouterdConfig_destroy(config2);


    return NULL;
}


char *all_tests(){

    mu_suite_start();

    mu_run_test(test_RouterdConfig_create);
    mu_run_test(test_RouterdConfig_debug_active);

    return NULL;
}

RUN_TESTS(all_tests);