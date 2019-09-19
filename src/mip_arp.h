#include <stdint.h>

struct mip_arp_cache *create_cache();

int append_to_cache(struct mip_arp_cache *cache, uint8_t address, uint8_t interface[]);

void print_cache(struct mip_arp_cache *cache);

int complete_mip_arp(struct sockaddr_ll *so_name, int num_interfaces, int raw_socket_fd, uint8_t mip_address);