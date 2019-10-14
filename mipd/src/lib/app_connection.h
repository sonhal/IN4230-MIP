#ifndef _APP_CONNECTION_H
#define _APP_CONNECTION_H

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
#include <linux/if_packet.h>
#include <net/ethernet.h> /* the L2 protocols */

#include "packaging/mip_package.h"
#include "server.h"

LocalSocket *LocalSocket_create(char *path, enum __socket_type socket_type);

void LocalSocket_destroy(LocalSocket *local_socket);

int create_domain_socket();

int setup_domain_socket(struct sockaddr_un *so_name, char *socket_name, unsigned int socket_name_size);

int app_server(int so, char *socket_name, int socket_name_size);

int parse_domain_socket_request(char *buffer, uint8_t *mip_addr, char *message);

struct ping_message *parse_ping_request(BYTE *buffer);

int read_from_domain_socket(int socket_fd, BYTE *buffer, size_t buffer_size);

MIPPackage *create_queueable_ping_message_MIPPackage(struct ping_message *message);

int handle_MIPPackage_for_application(MIPDServer *server, MIPPackage *received_package);

#endif