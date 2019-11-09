#ifndef _BOUND_SOCKET_H
#define _BOUND_SOCKET_H

#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <sys/un.h>		/* struct sockaddr_un */

#include "dbg.h"

typedef struct BoundSocket {
    char *file_path;
    int socket_fd;
    struct sockaddr_un *so_name;
} BoundSocket;

BoundSocket *BoundSocket_create(char *file_path);

int BoundSocket_connect(BoundSocket *b_socket);

void BoundSocket_destroy(BoundSocket *b_socket);

#endif _BOUND_SOCKET_H