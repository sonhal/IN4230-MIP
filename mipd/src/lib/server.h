#include <stdint.h>

struct server_self {
    int listening_domain_socket;
    int connected_domain_socket;
    struct interface_table *i_table;
    struct mip_arp_cache *cache;
    int debug_enabled;
};

 #define server_log(SELF, M, ...) if((SELF->debug_enabled)) printf(\
    "[SERVER LOG]" M "\n", ##__VA_ARGS__)

 #define server_log_received_packet(SELF, HEADER) server_log(SELF, \
    "[PACKAGE RECEIVED] from mip_addr: %d\t tra: %d\tpayload length: %d\n", (HEADER)->src_addr, (HEADER)->tra, (HEADER)->payload_len)

void destroy_server_self(struct server_self *self);

struct server_self *init_server_self(int listening_domain_socket, struct interface_table *table, int debug_enabled, long cache_update_freq_milli);

int start_server(struct server_self *self, int epoll_fd, struct epoll_event *events, int event_num, int read_buffer_size, int timeout);
