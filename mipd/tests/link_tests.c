#include <unistd.h>		/* read, close, unlink */
#include <unistd.h>		/* read, close, unlink */
#include <sys/socket.h>
#include <unistd.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <stdint.h>
#include <stdlib.h> 
#include <netinet/in.h> 

#include "../../commons/src/dbg.h"
#include "minunit.h"
#include "../src/lib/link.h"
#include "../src/lib/packaging/mip_header.h"
#include "../src/lib/packaging/mip_packet.h"


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


char *test_sendto_mip_packet(){
    int n = 10;
    int interface_n = 0;
    int raw_socket = 0;
    int rc = 0;
    char *buffer = calloc(1024 , sizeof(char));

    struct sockaddr_ll *so_name = calloc(n, sizeof(struct sockaddr_ll));
    //struct sockaddr_ll **so_name_col = &so_name; 
    interface_n = collect_intefaces(so_name, n);
    raw_socket = setup_raw_socket();
    rc = bind(raw_socket, &so_name[0], sizeof(struct sockaddr_ll));
    struct ether_frame *e_frame = create_ethernet_frame(&so_name->sll_addr, &so_name[0]);
    struct mip_header *m_header = create_arp_request_mip_header(255);
    const BYTE *message = "HELLO";
    struct mip_packet *packet = create_mip_packet(e_frame, m_header, message, strlen("Message"));
    
    rc = sendto_raw_mip_packet(raw_socket, &so_name[0], packet);
    mu_assert(rc != -1, "Failed to send packet");

    //rc = recv(raw_socket, buffer, 1024, 0);
    mu_assert(rc != -1, "Failed to receive packet");

    printf("Received message from loopback: %s", buffer);

    free(buffer);
    free(so_name);
    return NULL;
}


char *all_tests(){

    mu_suite_start();

    mu_run_test(test_print_interface_list);
    mu_run_test(test_collect_interfaces);
    //mu_run_test(test_sendto_mip_packet); test requires sudo

    return NULL;
}

RUN_TESTS(all_tests);