#include <unistd.h>		/* read, close, unlink */
#include "../src/dbg.h"
#include "minunit.h"

#include "../src/mip_arp.c"


char *test_create_cache(){
    struct mip_arp_cache *cache = create_cache();

    mu_assert(cache != NULL, "cache is NULL");
    free(cache);
    return NULL;
}


char *test_append_cache(){
    struct mip_arp_cache *cache = create_cache();
    uint8_t interface[] = {0xaf, 0xff, 0xff, 0x00, 0xff, 0xff};
    int index = -1;
    index = append_to_cache(cache, 255, &interface);
    index = append_to_cache(cache, 128, &interface);

    mu_assert(index == 1, "index is not 1");
    mu_assert(cache->entries[1].address == 128, "entry at index 1 should have address 128");
    free(cache);
    return NULL;
}

char *test_print_cache(){
    struct mip_arp_cache *cache = create_cache();
    uint8_t interface[] = {0xff, 0xff, 0x00, 0xff, 0xff, 0xff};
    int index = -1;
    index = append_to_cache(cache, 255, &interface);
    uint8_t interface2[] = {0xaf, 0xff, 0xff, 0xff, 0xff, 0xff};
    index = append_to_cache(cache, 128, &interface2);

    print_cache(cache);
    free(cache);
    return NULL;
}


char *all_tests(){

    mu_suite_start();

    mu_run_test(test_create_cache);
    mu_run_test(test_append_cache);
    mu_run_test(test_print_cache);

    return NULL;
}

RUN_TESTS(all_tests);