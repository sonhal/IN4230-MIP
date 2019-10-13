#include <unistd.h>

#include "../../commons/src/dbg.h"
#include "minunit.h"
#include "../src/lib/mip_route_table.h"

char *test_MIPRouteTable_create() {
    MIPRouteTable *table = MIPRouteTable_create(1);
    mu_assert(table != NULL, "Table pointer should not be NULL");
    MIPRouteTable_destroy(table);
    return NULL;
}

char *test_MIPRouteTable_update() {
    MIP_ADDRESS result = 0;
    int rc = 0;

    MIPRouteTable *table = MIPRouteTable_create();
    rc = MIPRouteTable_update(table, 1, 1, 0);
    result = MIPRouteTable_get_next_hop(table, 1);

    mu_assert(result == 1, "Wrong next hop returned");
    MIPRouteTable_destroy(table);

    return NULL;
}

char *test_MIPRouteTable_dvr(){
    MIPRouteTable *table1 = MIPRouteTable_create();
    MIPRouteTable *table2 = MIPRouteTable_create();
    table2->table_address = 2;

    MIPRouteTable_update(table1, 1, 255, 0);
    MIPRouteTable_update(table1, 2, 2, 1);
    MIPRouteTable_update(table1, 3, 3, 5);

    MIPRouteTable_update(table2, 2, 255, 0);
    MIPRouteTable_update(table2, 1, 1, 1);
    MIPRouteTable_update(table2, 3, 3, 1);

    MIPRouteTable_update_routing(table1, table2);

    printf("table1:\n");
    MIPRouteTable_print(table1);
    mu_assert(MIPRouteTable_get_next_hop(table1, 3) == 2, "Wrong route to node 3");

    MIPRouteTable_destroy(table1);
    MIPRouteTable_destroy(table2);

    return NULL;
}

char *test_MIPRouteTable_print(){
    MIPRouteTable *table = MIPRouteTable_create();

    MIPRouteTable_update(table, 110, 255, 1);
    MIPRouteTable_update(table, 25, 3, 5);
    MIPRouteTable_update(table, 40, 3, 1);

    MIPRouteTable_print(table);

    MIPRouteTable_destroy(table);

    return NULL;
}


char *test_MIPRouteTable_create_package(){
    MIPRouteTable *table = MIPRouteTable_create();

    MIPRouteTable_update(table, 1, 255, 0);
    MIPRouteTable_update(table, 2, 2, 1);
    MIPRouteTable_update(table, 3, 3, 5);
    MIPRouteTable_update(table, 4, 3, 1);


    MIPRouteTablePackage *package = MIPRouteTable_create_package(table);
    mu_assert(package != NULL, "TablePackage is NULL");
    mu_assert(package->entries[0].destination == 1 && package->entries[0].cost == 0, "Wrong destination");
    mu_assert(package->entries[3].destination == 4 && package->entries[3].cost == 1, "Wrong destination");

    printf("Size of package: %d\n", sizeof(MIPRouteTablePackage));

    MIPRouteTable *table2 = MIPRouteTablePackage_create_table(package);
    mu_assert(table2 != NULL, "New table is NULL");
    mu_assert(MIPRouteTable_get_next_hop(table, 4) == MIPRouteTable_get_next_hop(table2, 4), "Failed to convert package to table");

    MIPRouteTable_destroy(table);
    MIPRouteTable_destroy(table2);
    MIPRouteTablePackage_destroy(package);

    return NULL;
}

char *test_MIPRouteTable_remove_old_entries(){
    MIPRouteTable *table = MIPRouteTable_create();
    table->entry_max_age_milli = 0;

    MIPRouteTable_update(table, 1, 255, 0);
    MIPRouteTable_update(table, 2, 2, 1);
    MIPRouteTable_update(table, 3, 3, 5);
    MIPRouteTable_update(table, 4, 3, 1);

    sleep(1);

    MIPRouteTable_remove_old_entries(table);

    mu_assert(table->entries->count == 1, "Number of addresses in table is not 1");

    MIPRouteTable_destroy(table);

    return NULL;
}


char *all_tests(){

    mu_suite_start();

    mu_run_test(test_MIPRouteTable_create);
    mu_run_test(test_MIPRouteTable_update);
    mu_run_test(test_MIPRouteTable_dvr);
    mu_run_test(test_MIPRouteTable_print);
    mu_run_test(test_MIPRouteTable_create_package);
    mu_run_test(test_MIPRouteTable_remove_old_entries);


    return NULL;
}

RUN_TESTS(all_tests);