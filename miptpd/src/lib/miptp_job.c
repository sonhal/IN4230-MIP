#include <stdlib.h>
#include <string.h>

#include "../../../commons/src/dbg.h"
#include "../../../commons/src/time_commons.h"

#include "miptp_job.h"

SlidingWindow SlidingWindow_create(uint16_t sequence_base, uint16_t window_size){
    SlidingWindow window;
    window.sequence_base = sequence_base;
    window.sequence_max = window_size + 1;
    window.window_size = window_size;
    
    return window;
}

MIPTPJob *MIPTPJob_create(BYTE *data, uint16_t data_size, unsigned long timeout_len){
    MIPTPJob *job = calloc(1, sizeof(MIPTPJob));
    check_mem(job);

    job->sliding_window = SlidingWindow_create(0, WINDOW_SIZE);
    job->timeout_len = timeout_len;
    job->data_size = data_size;
    job->data = calloc(job->data_size, sizeof(BYTE));
    check_mem(job->data);

    memcpy(job->data, data, job->data_size);
    job->last_ack = get_now_milli();
    
    return job;

    error:
        log_err("Failed to create MIPTTPJob");
        return NULL;
}

void MIPTPJob_destroy(MIPTPJob *job){
    if(job){
        if(job->data){
            free(job->data);
        }
        free(job);
    }
}