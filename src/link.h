
int last_inteface(struct sockaddr_ll *so_name);

int  send_raw_packet(int sd, struct sockaddr_ll *so_name, char *message, int message_length);

