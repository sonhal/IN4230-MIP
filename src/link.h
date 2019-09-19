#include <linux/if_packet.h>	/* AF_PACKET */

char *macaddr_str(struct sockaddr_ll *sa);

int last_inteface(struct sockaddr_ll *so_name);

int collect_intefaces(struct sockaddr_ll *so_name, int buffer_n);

int receive_raw_packet(int sd, char *buf, size_t len);

int send_raw_package(int sd, struct sockaddr_ll *so_name, char *message, int message_length);

int send_raw_package_mip(int sd, struct sockaddr_ll *so_name, struct mip_header *mip_header);

int setup_raw_socket();
