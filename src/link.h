#include <linux/if_packet.h>	/* AF_PACKET */


int last_inteface(struct sockaddr_ll *so_name);

int receive_raw_packet(int sd, char *buf, size_t len);

int send_raw_package(int sd, struct sockaddr_ll *so_name, char *message, int message_length);

int setup_raw_socket();
