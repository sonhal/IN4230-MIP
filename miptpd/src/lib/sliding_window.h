#ifndef _MIPTP_SLIDING_WINDOW_H
#define _MIPTP_SLIDING_WINDOW_H

#include <stdint.h>

typedef struct SlidingWindow
{
    uint16_t window_size;
    uint16_t sequence_base;
    uint16_t sequence_max;
} SlidingWindow;

SlidingWindow SlidingWindow_create(uint16_t sequence_base, uint16_t window_size);

// Updates the sliding widnow with the new seqence number
void SlidingWindow_update(SlidingWindow *window, uint16_t seqence_nr);

#endif