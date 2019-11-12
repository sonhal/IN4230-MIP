#ifndef _MIPTP_RECEIVE_JOB_H
#define _MIPTP_RECEIVE_JOB_H

#include <stdint.h>

#include "../../../commons/src/client_package.h"
#include "../../../commons/src/definitions.h"
#include "../../../commons/src/queue.h"

#include "miptp_package.h"
#include "sliding_window.h"

#define MAX_DATA_BATCH_SIZE_BYTES 1492

enum MIPTPReceiveJobStatus {RJOB_WAITING_FOR_CONNECTION = 1, RJOB_WAITING_FOR_PACKAGE = 2, RJOB_ACK_READY = 3 };

typedef struct MIPTPReceiveJob
{
    uint16_t port: 14;
    MIP_ADDRESS sender;
    enum MIPTPReceiveJobStatus status;
    unsigned long timeout;
    unsigned long last_package_time;
    BYTE *data;
    uint16_t data_received;
    uint16_t data_size;
    uint16_t request_nr;
} MIPTPReceiveJob;


MIPTPReceiveJob *MIPTPReceiveJob_create(uint16_t port, unsigned long timeout);

void MIPTPReceiveJob_destroy(MIPTPReceiveJob *job);

Queue *MIPTPReceiveJob_next_packages(MIPTPReceiveJob *job);

MIPTPPackage *MIPTPReceiveJob_next_package(MIPTPReceiveJob *job, uint16_t sequence_nr);

int MIPTPReceiveJob_receive_package(MIPTPReceiveJob *job, MIP_ADDRESS sender, MIPTPPackage *package);

// returns 1 of the job is complete or timed out, 0 if it is still active
int MIPTPReceiveJob_finished(MIPTPReceiveJob *job);

ClientPackage *MIPTPReceiveJob_result(MIPTPReceiveJob *job);

int MIPTPReceiveJob_is_complete(MIPTPReceiveJob *job);

#define MIPTPReceiveJob_is_waiting_for_connection(J) ((J)->status == RJOB_WAITING_FOR_CONNECTION)
#define MIPTPReceiveJob_is_waiting_for_data_size(J) ((J)->status == RJOB_WAITING_FOR_DATA_SIZE)
#define MIPTPReceiveJob_is_waiting_for_data_package(J) ((J)->status == RJOB_WAITING_FOR_DATA_PACKAGE)
#define MIPTPReceiveJob_is_ack_ready(J) ((J)->status == RJOB_ACK_READY)
#define MIPTPReceiveJob_set_status(J, S) ((J)->status = S)

#endif