#include <unistd.h>		/* read, close, unlink */

#include "../src/dbg.h"
#include "minunit.h"


#include "../src/link.c"


char *test_print_interface_list(){
    struct sockaddr_ll *so_name = calloc(1, sizeof(struct sockaddr_ll));
    last_inteface(so_name);
    free(so_name);
    return NULL;
}

char *test_collect_interfaces(){
    int n = 10;
    int interface_n = 0;
    struct sockaddr_ll *so_name = calloc(n, sizeof(struct sockaddr_ll));
    //struct sockaddr_ll **so_name_col = &so_name; 
    interface_n = collect_intefaces(so_name, n);
    
    int i = 0;
    for ( i = 0; i < interface_n; i++)
    {
        char *str;
        str = macaddr_str(&so_name[i]);
        printf("interface mac: %s\n", str);
    }
    

    free(so_name);
    return NULL;
}


char *all_tests(){

    mu_suite_start();

    mu_run_test(test_print_interface_list);
    mu_run_test(test_collect_interfaces);

    return NULL;
}

RUN_TESTS(all_tests);