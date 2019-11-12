#include <stdlib.h>

#include "../../../commons/src/dbg.h"
#include "../../../commons/src/definitions.h"
#include "../../../commons/src/time_commons.h"

#include "miptp_receive_job.h"

uint16_t MIPTPReceiveJob_get_data_size_from_package(MIPTPPackage *package);
int MIPTPReceiveJob_append_data(MIPTPReceiveJob *job, BYTE *data, uint16_t data_size);

MIPTPReceiveJob *MIPTPReceiveJob_create(uint16_t port, unsigned long timeout){
    MIPTPReceiveJob *job = calloc(1, sizeof(MIPTPReceiveJob));
    check_mem(job);
    job->port = port;
    job->timeout = timeout;
    job->data = calloc(MAX_DATA_SIZE_BYTES, sizeof(BYTE));
    check_mem(job->data);
    job->data_size = 0;
    job->request_nr = 0;
    job->last_package_time = 0;

    return job;

    error:
        MIPTPReceiveJob_destroy(job);
        return NULL;
}

void MIPTPReceiveJob_destroy(MIPTPReceiveJob *job){
    if(job){
        if(job->data) free(job->data);
        free(job);
    }
}

Queue *MIPTPReceiveJob_next_packages(MIPTPReceiveJob *job){
    Queue *packages = Queue_create();
    if(!MIPTPReceiveJob_is_ack_ready(job)) return packages;

    MIPTPPackage *ack_package = MIPTPReceiveJob_next_package(job, job->request_nr);
    check(ack_package != NULL, "Failed to create ack package for reqnr: %d", job->request_nr);
    MIPTPReceiveJob_set_status(job, RJOB_WAITING_FOR_PACKAGE);

    return ack_package;

    error:
        return NULL;
}

MIPTPPackage *MIPTPReceiveJob_next_package(MIPTPReceiveJob *job, uint16_t sequence_nr){
    MIPTPPackage *package = MIPTPPackage_create(job->port, sequence_nr, NULL, 0);
    check(package != NULL, "Failed to create MIPTPPackage");

    return package;

    error:
        return NULL;
}


// Passes the package to the job for handling, frees the MIPTPPackage package pointer
int MIPTPReceiveJob_receive_package(MIPTPReceiveJob *job, MIP_ADDRESS sender, MIPTPPackage *package){
    check(job != NULL, "Invalid argument, job is NULL");
    check(package != NULL, "Invalid argument, package is NULL");
    int rc = 0;

    if(MIPTPReceiveJob_is_waiting_for_connection(job)){
        job->sender = sender;
        uint16_t data_size = MIPTPReceiveJob_get_data_size_from_package(package);
        job->data_size = data_size;
        MIPTPReceiveJob_set_status(job, RJOB_WAITING_FOR_PACKAGE);
    } else
    {
        if(!MIPTPReceiveJob_is_complete(job)){
            rc = MIPTPReceiveJob_append_data(job, package->data, package->data_size);
            check(rc != -1, "Failed")
        }
    }

    return 1;

    MIPTPPackage_destroy(package);
    error:
        MIPTPPackage_destroy(package);
        return -1;
}


uint16_t MIPTPReceiveJob_get_data_size_from_package(MIPTPPackage *package){
    uint16_t data_size = 0;
    memcpy(&data_size, package->data, sizeof(uint16_t));
    return data_size;
}


int MIPTPReceiveJob_append_data(MIPTPReceiveJob *job, BYTE *data, uint16_t data_size){
    check(job != NULL, "Invalid argument, job is NULL");
    check(data != NULL, "Invalid argument, data is NULL");

    memcpy(&job->data[job->data_received], data, data_size);

    error:
        return -1;
}

// returns 1 of the job is complete, 0 if it is still active
int MIPTPReceiveJob_is_complete(MIPTPReceiveJob *job){
    return job->data_received == job->data_size;
}

// returns 1 of the job is complete, 0 if it is still active
int MIPTPReceiveJob_timedout(MIPTPReceiveJob *job){
    unsigned long current_time = get_now_milli();
    return (current_time - job->last_package_time) > job->timeout;
}


ClientPackage *MIPTPReceiveJob_result(MIPTPReceiveJob *job){
    check(MIPTPReceiveJob_is_complete(job), "Invalid state, MIPTPReceiveJob is not complete");

    ClientPackage *package = ClientPackage_create(0 ,0 , MIPTP_SENDER, job->data, job->data_received);
    check(package != NULL, "Failed to create ClientPackage");

    return package;

    error:
        return NULL;
}
