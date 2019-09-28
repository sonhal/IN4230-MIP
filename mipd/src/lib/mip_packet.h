#include <stdlib.h>

#include "link.h"
#include "mip.h"


struct mip_packet
{
    struct ether_frame e_frame;
    struct mip_header m_header;
    char *message;
    size_t message_size;
};

struct mip_packet *create_mip_packet(const struct ether_frame *e_frame, const struct mip_header *m_header, const char *message);

void destroy_mip_packet(struct mip_packet *packet);