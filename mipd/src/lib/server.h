#ifndef _MIPDServer_H
#define _MIPDServer_H
#include <stdint.h>

#include "../../../commons/src/queue.h"
#include "packaging/mip_package.h"

typedef struct ForwardQueueEntry {
    MIP_ADDRESS destination;
    MIPPackage *package;
    long age_milli;
} ForwardQueueEntry;

typedef struct ForwardQueue {
    Queue *queue;
} ForwardQueue;


typedef struct LocalSocket {
    int listening_socket_fd;
    int connected_socket_fd;
    struct sockaddr_un *so_name;
} LocalSocket;

typedef struct MIPDServer {
    LocalSocket *app_socket;
    LocalSocket *route_socket;
    LocalSocket *forward_socket;
    struct interface_table *i_table;
    struct mip_arp_cache *cache;
    ForwardQueue *forward_queue;
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