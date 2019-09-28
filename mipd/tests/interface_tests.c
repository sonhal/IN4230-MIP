#include <unistd.h>		/* read, close, unlink */
#include "../../commons/src/dbg.h"
#include "minunit.h"

#include <sys/socket.h>
#include <unistd.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <stdint.h>
#include <stdlib.h> 
#include <netinet/in.h> 

#include "../src/lib/interface.h"

char *test_create_interface_table(){
    struct interface_table *table = create_interface_table();
    mu_assert( table != NULL, "Table is a null ptr");
    return NULL;
}


char *test_append_interface_table(){
    int num_i = 0;
    int rc = 0;
    int i = 0;
    struct sockaddr_ll *so_names = calloc(10, SOCKET_ADDR_SIZE);
    struct interface_table *table = create_interface_table();
    num_i = collect_intefaces(so_names,10);

    for(i = 0; i <= rc; i++){
        rc = append_interface_table(table, &so_names[i]);
        mu_assert(rc != -1, "Failed to append interface");
    }
    return NULL;
}

char *test_print_interface_table(){
    int num_i = 0;
    int rc = 0;
    int i = 0;
    struct sockaddr_ll *so_names = calloc(10, SOCKET_ADDR_SIZE);
    struct interface_table *table = create_interface_table();
    num_i = collect_intefaces(so_names,10);

    for(i = 0; i < num_i; i++){
        rc = append_interface_table(table, &so_names[i]);
        mu_assert(rc != -1, "Failed to append interface");
    }
    print_interface_table(table);

    free(so_names);
    return NULL;
}

char *test_get_interface_table(){
    int num_i = 0;
    int rc = 0;
    int i = 0;
    struct sockaddr_ll *so_names = calloc(10, SOCKET_ADDR_SIZE);
    struct interface_table *table = create_interface_table();
    num_i = collect_intefaces(so_names,10);

    for(i = 0; i < num_i; i++){
        rc = append_interface_table(table, &so_names[i]);
        mu_assert(rc != -1, "Failed to append interface");
    }
    struct sockaddr_ll *test = calloc(1, SOCKET_ADDR_SIZE);
    rc = get_interface(table, test, &so_names[4].sll_addr);
    mu_assert(rc != -1, "Failed to find address");
    printf("getted address: %s\n", macaddr_str(test));
    printf("expected address: %s\n", macaddr_str(&so_names[4]));
    printf("at index: %d\n", rc);
    
    free(so_names);
    free(test);
    return NULL;
}

char *test_create_loaded_interface_table(){
    struct interface_table *table = create_loaded_interface_table();
    mu_assert(table != NULL, "table pointer is NULL");
    print_interface_table(table);
    free(table);
    return NULL;
}


char *all_tests(){

    mu_suite_start();

    mu_run_test(test_create_interface_table);
    mu_run_test(test_append_interface_table);
    mu_run_test(test_print_interface_table);
    mu_run_test(test_get_interface_table);
    mu_run_test(test_create_loaded_interface_table);

    return NULL;
}

RUN_TESTS(all_tests);