#ifndef _MIPTP_JOB_H
#define _MIPTP_JOB_H

#include "../../../commons/src/definitions.h"
#include "../../../commons/src/queue.h"

#include "miptp_package.h"

#define WINDOW_SIZE 10
#define MAX_DATA_BATCH_SIZE_BYTES 1492

typedef struct SlidingWindow
{
    uint16_t window_size;
    uint16_t sequence_base;
    uint16_t sequence_max;
} SlidingWindow;

typedef struct MIPTPJob
{
    uint16_t port: 14;
    unsigned long timeout;
    unsigned long last_ack;
    BYTE *data;
    uint16_t data_size;
    SlidingWindow sliding_window;

} MIPTPJob;


MIPTPJob *MIPTPJob_create(BYTE *data, uint16_t data_size, unsigned long timeout);

void MIPTPJob_destroy(MIPTPJob *job);

Queue *MIPTPJob_next_packages(MIPTPJob *job);

MIPTPPackage *MIPTPJob_next_package(MIPTPJob *job, uint16_t sequence_nr);

int MIPTPJob_receive_ack(MIPTPJob *job, uint16_t sequence_nr);

// returns 1 of the job is complete or timed out, 0 if it is still active
int MIPTPJob_finished(MIPTPJob *job);

#endif