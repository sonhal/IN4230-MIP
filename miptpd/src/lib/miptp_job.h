#ifndef _MIPTP_JOB_H
#define _MIPTP_JOB_H

#include "../../../commons/src/definitions.h"
#include "../../../commons/src/queue.h"

#include "miptp_package.h"

#define WINDOW_SIZE 10

typedef struct SlidingWindow
{
    uint16_t window_size;
    uint16_t sequence_base;
    uint16_t sequence_max;
} SlidingWindow;

typedef struct MIPTPJob
{
    unsigned long timeout_len;
    unsigned long last_ack;
    BYTE *data;
    uint16_t data_size;
    SlidingWindow sliding_window;

} MIPTPJob;


MIPTPJob *MIPTPJob_create(BYTE *data, uint16_t data_size, unsigned long timeout_len);

void MIPTPJob_destroy(MIPTPJob *job);

Queue *MIPTPJob_next_packages(MIPTPJob *job);

Queue *MIPTPJob_receive_ack(MIPTPJob *job, MIPTPPackage *package);

int MIPTPJob_finished(MIPTPJob *job);

#endif