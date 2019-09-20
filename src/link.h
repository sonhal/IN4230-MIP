#include <linux/if_packet.h>	/* AF_PACKET */

struct ether_frame {
    uint8_t dst_addr[6];
    uint8_t src_addr[6];
    uint8_t eth_proto[2];
    uint8_t contents[0];
} __attribute__((packed));

char *macaddr_str(struct sockaddr_ll *sa);

int last_inteface(struct sockaddr_ll *so_name);

int collect_intefaces(struct sockaddr_ll *so_name, int buffer_n);

int receive_raw_packet(int sd, char *buf, size_t len);

int receive_raw_mip_packet(int sd, struct ether_frame  *frame_hdr, struct sockaddr_ll *so_name, struct mip_header *header);

int send_raw_package(int sd, struct sockaddr_ll *so_name, char *message, int message_length);

int send_raw_mip_packet(int sd, struct sockaddr_ll *so_name, struct ether_frame *frame_hdr, struct mip_header *mip_header);

int setup_raw_socket();

int complete_mip_arp(struct interface_table *table);

struct ether_frame *create_response_ethernet_frame(struct ether_frame *request_ethernet);

struct ether_frame *create_ethernet_frame(int8_t *dest[], struct sockaddr_ll *so_name);