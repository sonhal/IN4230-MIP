#include "../../commons/src/dbg.h"
#include "../../commons/src/domain_socket.h"
#include "minunit.h"

#include "../src/lib/cli.h"
#include "../src/lib/router_server.h"

int argc = 6;
char *argv[] = {"routerd", "-d", "my_route_socket", "my_forward_socket", "10", "20"};
struct sockaddr_un routing_so_name;
struct sockaddr_un forwarding_so_name;


char *test_RouterServer_create() {
    RouterdConfig *config = RouterdConfig_create(argc, argv);
   
    RouterServer  *server = RouterServer_create(config);
    mu_assert(server != NULL, "server is NULL");
    mu_assert(RouterServer_is_debug_active(server), "server should have debug active");

    RouterdConfig_destroy(config);
    RouterServer_destroy(server);

    return NULL;
}


char *test_RouterServer_init(){
    int rc = 0;
    RouterdConfig *config = RouterdConfig_create(argc, argv);
    RouterServer  *server = RouterServer_create(config);


    setup_domain_socket(&routing_so_name, argv[2], strlen(argv[2]));

    setup_domain_socket(&forwarding_so_name, argv[3], strlen(argv[3]));

    rc = RouterServer_init(server);
    mu_assert(rc != -1, "Failed to init route server");
    mu_assert(server->epoll_fd != -1, "epoll is not setuped");

    RouterdConfig_destroy(config);
    RouterServer_destroy(server);

}

char *test_RouterServer_run(){
    int rc = 0;
    RouterdConfig *config = RouterdConfig_create(argc, argv);
    RouterServer  *server = RouterServer_create(config);


    //int r_fd = setup_domain_socket(&routing_so_name, argv[2], strlen(argv[2]));

    //int f_fd = setup_domain_socket(&forwarding_so_name, argv[3], strlen(argv[3]));

    rc = RouterServer_init(server);
    mu_assert(rc != -1, "Failed to init route server");

    rc = RouterServer_run(server);

    RouterdConfig_destroy(config);
    RouterServer_destroy(server);    
}


char *all_tests(){

    mu_suite_start();

    mu_run_test(test_RouterServer_create);
    //mu_run_test(test_RouterServer_init);
    mu_run_test(test_RouterServer_run);

    if(routing_so_name.sun_path)unlink(routing_so_name.sun_path);
    if(forwarding_so_name.sun_path)unlink(forwarding_so_name.sun_path);

    return NULL;
}

RUN_TESTS(all_tests);