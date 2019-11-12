#include <stdlib.h>
#include <string.h>

#include "../../../commons/src/dbg.h"
#include "../../../commons/src/time_commons.h"

#include "miptp_send_job.h"

MIPTPSendJob *MIPTPSendJob_create(uint16_t port, MIP_ADDRESS destination, BYTE *data, uint16_t data_size, unsigned long timeout){
    MIPTPSendJob *job = calloc(1, sizeof(MIPTPSendJob));
    check_mem(job);

    job->sliding_window = SlidingWindow_create(0, WINDOW_SIZE);
    job->port = port;
    job->timeout = timeout;
    job->data_size = data_size;
    job->destination = destination;
    job->data = calloc(job->data_size, sizeof(BYTE));
    check_mem(job->data);

    memcpy(job->data, data, job->data_size);
    job->last_ack = get_now_milli();
    
    return job;

    error:
        log_err("Failed to create MIPTTPJob");
        return NULL;
}

void MIPTPSendJob_destroy(MIPTPSendJob *job){
    if(job){
        if(job->data){
            free(job->data);
        }
        free(job);
    }
}


// Returns a Queue of the next MIPTPackages in the window
// Caller takes ownership of the pointer
Queue *MIPTPSendJob_next_packages(MIPTPSendJob *job){
    uint16_t i = job->sliding_window.sequence_base;
    uint16_t max = job->sliding_window.sequence_max;
    Queue *packages = Queue_create();

    for(; i < max; i++){
        MIPTPPackage *package = MIPTPSendJob_next_package(job, i);
        if(package == NULL) break;

        Queue_send(packages, package);
    }

    return packages;
}


// Bites of a chunk of the data positioned by the sequence number.
// Returns a MIPTPPackage pointer that the caller takes responsibility over.
// Returns a NULL pointer if there are no packages to create.
MIPTPPackage *MIPTPSendJob_next_package(MIPTPSendJob *job, uint16_t sequence_nr){
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


// Receives and handles a MIPTPPackage from the other MIPTP daemon.
// Returns 1 on success, 0 on failure
int MIPTPSendJob_receive_package(MIPTPSendJob *job, MIPTPPackage *package){
    SlidingWindow_update(&job->sliding_window, package->miptp_header.PSN);
    return 1;
}

ClientPackage *MIPTPSendJob_result(MIPTPSendJob* job){
    check(MIPTPSendJob_is_complete(job), "Invalid state, MIPTPSendJob is not complete");

    ClientPackage *result = ClientPackage_create(job->port, job->destination, MIPTP_RECEIVER,"OK", sizeof("OK"));
    check(result != NULL, "Failed to create ClientPackage");

    return result;

    error:
        return NULL;
}


// returns 1 of the job is complete, 0 if it is still active
int MIPTPSendJob_is_complete(MIPTPSendJob *job) {
    unsigned int num_packages = job->data_size / MAX_DATA_BATCH_SIZE_BYTES;
    if(job->data_size % MAX_DATA_BATCH_SIZE_BYTES) num_packages++;
    
    // seqence_base is index based, we have to deduct 1 from num packages when checking
    return job->sliding_window.sequence_base >= (num_packages - 1);
}



// Returns 1 if the job is timed out, 0 if it is still active
int MIPTPSendJob_is_timed_out(MIPTPSendJob *job){
    unsigned long current_milli = get_now_milli();
    return (current_milli - job->last_ack) > job->timeout;
}

