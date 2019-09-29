#ifndef _APP_CONNECTION_H
#define _APP_CONNECTION_H

#include <linux/if_packet.h>
#include <net/ethernet.h> /* the L2 protocols */

#include "packaging/mip_packet.h"

int create_domain_socket();

int setup_domain_socket(struct sockaddr_un *so_name, char *socket_name, unsigned int socket_name_size);

int app_server(int so, char *socket_name, int socket_name_size);

int parse_domain_socket_request(char *buffer, uint8_t *mip_addr, char *message);

struct ping_message *parse_ping_request(BYTE *buffer);
#endif