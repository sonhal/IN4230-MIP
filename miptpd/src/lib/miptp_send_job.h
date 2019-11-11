#ifndef _MIPTP_SEND_JOB_H
#define _MIPTP_SEND_JOB_H

#include "../../../commons/src/client_package.h"
#include "../../../commons/src/definitions.h"
#include "../../../commons/src/queue.h"

#include "miptp_package.h"
#include "sliding_window.h"

#define WINDOW_SIZE 10
#define MAX_DATA_BATCH_SIZE_BYTES 1492

typedef struct MIPTPSendJob
{
    uint16_t port: 14;
    unsigned long timeout;
    unsigned long last_ack;
    BYTE *data;
    uint16_t data_size;
    SlidingWindow sliding_window;
} MIPTPSendJob;


MIPTPSendJob *MIPTPSendJob_create(uint16_t port, BYTE *data, uint16_t data_size, unsigned long timeout);

void MIPTPSendJob_destroy(MIPTPSendJob *job);

Queue *MIPTPSendJob_next_packages(MIPTPSendJob *job);

MIPTPPackage *MIPTPSendJob_next_package(MIPTPSendJob *job, uint16_t sequence_nr);

int MIPTPSendJob_receive_package(MIPTPSendJob *job, MIPTPPackage *package);

// returns 1 of the job is complete, 0 if it is still active
int MIPTPSendJob_is_complete(MIPTPSendJob *job);

// Returns 1 if the job is timed out, 0 if it is still active
int MIPTPSendJob_is_timed_out(MIPTPSendJob *job);

#endif