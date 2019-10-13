
#ifndef _LINK_H
#define _LINK_H
#include <linux/if_packet.h>	/* AF_PACKET */
#include "packaging/mip_header.h"
#include "packaging/ether_frame.h"
#include "packaging/mip_package.h"
#include "interface.h"


int last_inteface(struct sockaddr_ll *so_name);

int sendto_raw_mip_package(int sd, struct sockaddr_ll *so_name, MIPPackage *package);

int recv_raw_mip_package(int sd, MIPPackage *package);

int setup_raw_socket();

#endif