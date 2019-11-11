#ifndef _MIPTP_RECEIVE_JOB_H
#define _MIPTP_RECEIVE_JOB_H

#include <stdint.h>

#include "../../../commons/src/client_package.h"
#include "../../../commons/src/definitions.h"
#include "../../../commons/src/queue.h"

#include "miptp_package.h"
#include "sliding_window.h"

#define MAX_DATA_BATCH_SIZE_BYTES 1492

typedef struct MIPTPReceiveJob
{
    uint16_t port: 14;
    unsigned long timeout;
    unsigned long last_package_time;
    BYTE *data;
    uint16_t data_size;
    uint16_t request_nr;
} MIPTPReceiveJob;


MIPTPReceiveJob *MIPTPReceiveJob_create(uint16_t port, unsigned long timeout);

void MIPTPReceiveJob_destroy(MIPTPReceiveJob *job);

Queue *MIPTPReceiveJob_next_packages(MIPTPReceiveJob *job);

MIPTPPackage *MIPTPReceiveJob_next_package(MIPTPReceiveJob *job, uint16_t sequence_nr);

int MIPTPReceiveJob_receive_package(MIPTPReceiveJob *job, MIPTPPackage *package);

// returns 1 of the job is complete or timed out, 0 if it is still active
int MIPTPReceiveJob_finished(MIPTPReceiveJob *job);

ClientPackage *MIPTPReceiveJob_response(MIPTPReceiveJob *job);

int MIPTPReceiveJob_is_complete(MIPTPReceiveJob *job);

#endif