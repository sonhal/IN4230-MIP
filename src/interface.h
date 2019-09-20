#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h> /* the L2 protocols */
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>		/* htons */
#include <ifaddrs.h>		/* getifaddrs */

#define MAC_ADDRESS_SIZE (6 * sizeof( unsigned char))
#define SOCKET_ADDR_SIZE sizeof(struct sockaddr_ll)

struct interface_record {
    unsigned char interface[8];
    struct sockaddr_ll *so_name;
    uint8_t mip_address;
    int raw_socket;
};

struct interface_table {
    struct interface_record interfaces[32];
    int size;
};

struct interface_table *create_interface_table();

int append_interface_table(struct interface_table *table, struct sockaddr_ll *so_name);

int interface_equal(uint8_t interface_1[6], uint8_t interface_2[6]);

int get_interface(struct interface_table *table, struct sockaddr_ll *so_name, uint8_t *get_interface[]);

void print_interface_table(struct interface_table *table);

int collect_intefaces(struct sockaddr_ll *so_name, int buffer_n);

struct interface_table *create_loaded_interface_table();

struct interface_table *apply_mip_addresses(const struct interface_table *table, uint8_t *mip_addresses[], int num_addresses);

int is_socket_in_table(struct interface_table *table, int fd);

int get_interface_pos_for_socket(struct interface_table *table, int fd);

char *macaddr_str(struct sockaddr_ll *sa);

char *macaddr_str_for_int_buff(char address[]);

int close_open_sockets_on_table_interface(struct interface_table *table);
