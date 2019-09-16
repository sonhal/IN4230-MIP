
int create_domain_socket();

int setup_domain_socket(struct sockaddr_un *so_name, char *socket_name, unsigned int socket_name_size);

int setup_raw_socket();

int app_server(int so, char *socket_name, int socket_name_size);