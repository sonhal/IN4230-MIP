#include <string.h>

#include "../../commons/src/dbg.h"
#include "minunit.h"

#include "../src/lib/miptp_package.h"

char *test_MIPTPPackage_serialize() {
    int byte_size = 0;
    MIPTPPackage *package = MIPTPPackage_create(16383, 1, "Hello", strlen("Hello"));
    mu_assert(package != NULL, "Failed to create MIPTPPackage");

    BYTE *s_package = calloc(sizeof(MIPTPPackage) + package->data_size, sizeof(BYTE));
    byte_size = MIPTPPackage_serialize(s_package, package);
    mu_assert(s_package != NULL, "Failed to serialize MIPTPPackage");
    MIPTPPackage_destroy(package);
    free(s_package);

    return NULL;
}

char *test_MIPTPPackage_deserialize() {
    int byte_size = 0;
    MIPTPPackage *package = MIPTPPackage_create(16383, 1, "Hello", strlen("Hello"));
    mu_assert(package != NULL, "Failed to create MIPTPPackage");

    BYTE *s_package = calloc(sizeof(MIPTPPackage) + package->data_size, sizeof(BYTE));
    byte_size = MIPTPPackage_serialize(s_package, package);
    mu_assert(s_package != NULL, "Failed to serialize MIPTPPackage");

    MIPTPPackage *package2 = MIPTPPackage_deserialize(s_package);
    mu_assert(package2 != NULL, "Failed to deserialize MIPTPPackage");
    mu_assert(package->miptp_header.PL == package2->miptp_header.PL, "PL is not correct after deseralization");
    mu_assert(package->miptp_header.port == package2->miptp_header.port, "port is not correct after deseralization");
    mu_assert(package->miptp_header.PSN == package2->miptp_header.PSN, "PSN is not correct after deseralization");
    mu_assert(package->data_size == package2->data_size, "data_size is not correct after deseralization");

    MIPTPPackage_destroy(package2);
    MIPTPPackage_destroy(package);
    free(s_package);

    return NULL;
}

char *test_MIPTPPackage_create_destroy() {
    MIPTPPackage *package = MIPTPPackage_create(16383, 1, "Hello", strlen("Hello"));
    mu_assert(package != NULL, "Failed to create MIPTPPackage");

    MIPTPPackage_destroy(package);

    return NULL;
}

char *test_calc_pl() {
    uint16_t data_size = 5;

    uint8_t result = 0;
    result = calc_pl(data_size);
    mu_assert(result == 3, "Wrong PL value calculated");

    result = 0;
    data_size = 7;
    result = calc_pl(data_size);
    mu_assert(result == 1, "Wrong PL value calculated");

    result = 0;
    data_size = 0;
    result = calc_pl(data_size);
    mu_assert(result == 0, "Wrong PL value calculated");

    return NULL;
}

char *all_tests(){

    mu_suite_start();

    mu_run_test(test_MIPTPPackage_create_destroy);
    mu_run_test(test_MIPTPPackage_serialize);
    mu_run_test(test_MIPTPPackage_deserialize);
    mu_run_test(test_calc_pl);

    return NULL;
}

RUN_TESTS(all_tests);