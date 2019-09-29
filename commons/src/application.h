#ifndef _APPLICATION_H
#define _APPLICATION_H

#include <stdint.h>

struct ping_message {
    uint8_t src_mip_addr;
    uint8_t dst_mip_addr;
    char content[32];
};

#endif