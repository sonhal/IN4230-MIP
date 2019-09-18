#include <linux/if_packet.h>	/* AF_PACKET */


int last_inteface(struct sockaddr_ll *so_name);

int  send_raw_packet(int sd, struct sockaddr_ll *so_name, char *message, int message_length);

int send_ether_frame_on_raw_socket(int sd, struct sockaddr_ll *so_name, char *message, int message_length);

int setup_raw_socket();
