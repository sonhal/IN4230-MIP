#include "sliding_window.h"

SlidingWindow SlidingWindow_create(uint16_t sequence_base, uint16_t window_size){
    SlidingWindow window;
    window.sequence_base = sequence_base;
    window.sequence_max = window_size + 1;
    window.window_size = window_size;
    
    return window;
}

// Updates
void SlidingWindow_update(SlidingWindow *window, uint16_t seqence_nr){
    // If you receive a request number where Rn > Sb 
    if(seqence_nr > window->sequence_base){
        window->sequence_max = (window->sequence_max - window->sequence_base) + seqence_nr;
        window->sequence_base = seqence_nr;
    }
}