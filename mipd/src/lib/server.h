#ifndef _MIPDServer_H
#define _MIPDServer_H
#include <stdint.h>

#include "app_connection.h"

typedef struct MIPDServer {
    LocalSocket *app_socket;
    LocalSocket *route_socket;
    LocalSocket *forward_socket;
    struct interface_table *i_table;
    struct mip_arp_cache *cache;
    int debug_enabled;
} MIPDServer;


 #define MIPDServer_log(SELF, M, ...) if((SELF->debug_enabled)) printf(\
    "[SERVER LOG]" M "\n", ##__VA_ARGS__)

 #define MIPDServer_log_received_package(SELF, HEADER) MIPDServer_log(SELF, \
    "[PACKAGE RECEIVED] from mip_addr: %d\t tra: %d\tpayload length: %d\n", (HEADER)->src_addr, (HEADER)->tra, (HEADER)->payload_len)

void MIPDServer_destroy(MIPDServer *self);

MIPDServer *MIPDServer_create(LocalSocket *app_socket, LocalSocket *route_socket, LocalSocket *forward_socket, struct interface_table *table, int debug_enabled, long cache_update_freq_milli);

int MIPDServer_run(MIPDServer *server, int epoll_fd, struct epoll_event *events, int event_num, int read_buffer_size, int timeout);

#endif