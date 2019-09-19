#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h> /* the L2 protocols */
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>		/* htons */
#include <ifaddrs.h>		/* getifaddrs */

#include "interface.h"
#include "dbg.h"

#define INTERFACE_BUFF_SIZE 10


struct interface_table *create_interface_table(){
    struct interface_table *table = calloc(1, sizeof(struct interface_table));
    return table;
};

int append_interface_table(struct interface_table *table, struct sockaddr_ll *so_name){
    int rc = 0;
    int  i = 0;
    memcpy(table->interfaces[table->size].interface, so_name->sll_addr , MAC_ADDRESS_SIZE);
    table->interfaces[table->size].so_name = so_name;
    table->size += 1;
    return 1;
}

int interface_equal(uint8_t interface_1[], uint8_t interface_2[]){
    int i = 0;
    for (i = 0; i < 6; i++){
        printf("%02hhx = %02hhx\n", interface_1[i], interface_2[i]);
        if(interface_1[i] != interface_2[i]){
            return 0;
        }
    }
    printf("\n");
    return i;
}

//Attempts to find and load sockaddr_ll pointer into provided pointer corresponding to get_interface argument
// Returns -1 if unsuccessfull. index of interface in table if successfull.
int get_interface(struct interface_table *table, struct sockaddr_ll *so_name, uint8_t *get_interface[]){
    int i = 0;

    printf("Table size: %d\n", table->size);
    for(i = 0; i < table->size; i++){
        printf("checking interface at: %d\n", i);
        if(interface_equal(get_interface, table->interfaces[i].interface)){
            memcpy(so_name, table->interfaces[i].so_name, SOCKET_ADDR_SIZE);
            return i;
        }
    }
    return -1;
}

void print_interface_table(struct interface_table *table){
    char *macaddr;
    int i = 0;
    printf("----- Interface table -----\n");
    for(i = 0; i < table->size; i++){
        macaddr = macaddr_str_for_int_buff(&table->interfaces[i].interface);
        printf("interface:%d\taddress: %s\n", i, macaddr);
        free(macaddr);
    }
}

// Returns number of collected interfaces
int collect_intefaces(struct sockaddr_ll *so_name, int buffer_n){
    int rc = 0;
    int i = 0;
    struct ifaddrs *ifaces, *ifp;
    struct sockaddr_ll *tmp;

    rc = getifaddrs(&ifaces);
    check(rc != -1, "Failed to get ip addresses");

    // Walk the list looking for the ifaces interesting to us
    for(ifp = ifaces; ifp != NULL; ifp = ifp->ifa_next){
        if(ifp->ifa_addr != NULL && ifp->ifa_addr->sa_family == AF_PACKET){
           tmp = (struct sockaddr_ll*)ifp->ifa_addr;
           /* Remove loopback interface so we dont message ourselves */
           if(tmp->sll_ifindex == 1) {
               continue;
           }

            // Copy the address info into out variable
           memcpy(&so_name[i], (struct sockaddr_ll*)ifp->ifa_addr, sizeof(struct sockaddr_ll));
           i += 1;
           check(i < buffer_n, "Buffer size surpassed in interface collection");
        }
    }
    
    free(ifaces);
    return i; // number of collected interfaces
 
    error:
         return -1;
}

// Returns a pointer to a interface table the caller takes ownership to free
struct interface_table *create_loaded_interface_table(){
    int num_i = 0;
    int rc = 0;
    int i = 0;
    struct sockaddr_ll *so_names = calloc(INTERFACE_BUFF_SIZE, SOCKET_ADDR_SIZE);
    struct interface_table *table = create_interface_table();
    num_i = collect_intefaces(so_names, INTERFACE_BUFF_SIZE);
    debug("Colleced %d interfaces", num_i);

    for(i = 0; i < num_i; i++){
        rc = append_interface_table(table, &so_names[i]);
        check(rc != -1, "Failed to append interface with index %d to interface table", i);
    }

    return table;

    error:
        free(so_names);
        free(table);
        return NULL;
}

char *macaddr_str(struct sockaddr_ll *sa){
    char *macaddr = calloc(6 * 2 + 6, sizeof(char));
    
    int i = 0;
    for (i = 0; i < 6; i++){
        char *buf = strdup(macaddr);

        sprintf(macaddr, "%s%02hhx%s",
                buf,
                sa->sll_addr[i],
                (i < 5) ? ":" : "");

        free(buf);
    }
    return macaddr;
}


char *macaddr_str_for_int_buff(char address[]){
    char *macaddr = calloc(6 * 2 + 6, sizeof(char));
    
    int i = 0;
    for (i = 0; i < 6; i++){
        char *buf = strdup(macaddr);

        sprintf(macaddr, "%s%02hhx%s",
                buf,
                address[i],
                (i < 5) ? ":" : "");

        free(buf);
    }
    return macaddr;
}