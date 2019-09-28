#include <stdint.h>

struct server_self {
    int listening_domain_socket;
    int connected_domain_socket;
    struct interface_table *i_table;
    struct mip_arp_cache *cache;
    int debug_enabled;
};


void destroy_server_self(struct server_self *self);

struct server_self *init_server_self(int listening_domain_socket, struct interface_table *table, int debug_enabled);

int start_server(struct server_self *self, int epoll_fd, struct epoll_event *events, int event_num, int read_buffer_size, int timeout);
