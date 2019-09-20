
struct epoll_event create_epoll_in_event(int fd);

int setup_epoll(struct epoll_event events_to_handle[], int event_num);

int epoll_loop(int epoll_fd, int local_domain_socket, int raw_socket, struct epoll_event *events, int event_num, int read_buffer_size, int timeout);

int add_to_table_to_epoll(int fd, struct interface_table *table);