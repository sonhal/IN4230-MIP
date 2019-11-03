#include <string.h>

#include "../../commons/src/dbg.h"
#include "minunit.h"

#include "../src/lib/miptp_app_controller.h"


char *test_MIPTPAppController_handle_new_package() {
    MIPTPAppController *app_controller = MIPTPAppController_create(2);
    mu_assert(app_controller != NULL, "Failed to create MIPTPAppController");
    

    return NULL;
}


char *test_MIPTPAppController_create() {
    MIPTPAppController *app_controller = MIPTPAppController_create(5);
    mu_assert(app_controller != NULL, "Failed to create MIPTPAppController");

    return NULL;
}

char *test_MIPTPAppController_destroy() {
    MIPTPAppController *app_controller = MIPTPAppController_create(5);
    MIPTPAppController_destroy(app_controller);
    MIPTPAppController *app_controller2 = MIPTPAppController_create(5);
    List_destroy(app_controller2->connections);
    app_controller2->connections = NULL; // Check that destroy does not blow up in this case
    MIPTPAppController_destroy(app_controller2);

    return NULL;
}


char *all_tests(){

    mu_suite_start();

    mu_run_test(test_MIPTPAppController_create);
    mu_run_test(test_MIPTPAppController_destroy);

    return NULL;
}

RUN_TESTS(all_tests);