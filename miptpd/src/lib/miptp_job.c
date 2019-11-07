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

// Returns a Queue of the next MIPTPackages in the window
// Caller takes ownership of the pointer
Queue *MIPTPJob_next_packages(MIPTPJob *job){
    uint16_t i = job->sliding_window.sequence_base;
    uint16_t max = job->sliding_window.sequence_max;
    Queue *packages = Queue_create();

    for(; i < max; i++){
       MIPTPPackage *package = MIPTPJob_next_package(job, i);
       Queue_send(packages, package);
    }

    return packages;
}


// Bites of a chunk of the data positioned by the sequence number.
// Returns a MIPTPPackage pointer that the caller takes responsibility over.
// Returns a NULL pointer if there are no packages to create.
MIPTPPackage *MIPTPJob_next_package(MIPTPJob *job, uint16_t sequence_nr){
    int period = MAX_DATA_BATCH_SIZE_BYTES;
    uint16_t index = sequence_nr * period;
    if(index > job->data_size){
        return NULL; // no more packages to create
    }

    // Decide if the limit of the data batch. Ensuring we are not reading out of the edge of the data
    uint16_t limit = job->data_size < (index + period) ? job->data_size: index + period;

    BYTE *batch = calloc(limit - index, sizeof(BYTE));
    memcpy(batch, &job->data[index], limit - index);

    MIPTPPackage *package = MIPTPPackage_create(job->port, sequence_nr, batch, limit - index);
    check(package != NULL, "Failed to create package for the batch");

    return package;

    error:
        log_err("Error occurred when trying to create next package");
        return NULL;
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