#ifndef _DOMAIN_SOCKET_H
#define _DOMAIN_SOCKET_H

#include <stdlib.h> 		/* malloc */
#include <stdio.h>		/* printf */
#include <string.h>		/* memset, strcmp, strncpy */
#include <sys/socket.h>		/* socket, bind, listen, accept */
#include <sys/un.h>		/* struct sockaddr_un */
#include <unistd.h>		/* read, close, unlink */
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>

#include "dbg.h"


int create_domain_socket();

// Sets up a socket with the given file name, returns socket fd on success, -1 on failure
int setup_domain_socket(struct sockaddr_un *so_name, char *socket_name, unsigned int socket_name_size);

// Setup connection to domain socket, returns a socket fd if successfull
int connect_to_domain_socket(char *socket_file);

#endif