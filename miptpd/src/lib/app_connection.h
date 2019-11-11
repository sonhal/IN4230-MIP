#ifndef _APP_CONNECTION_H
#define _APP_CONNECTION_H

#include <stdint.h>

#include "../../../commons/src/list.h"
#include "../../../commons/src/definitions.h"
#include "../../../commons/src/client_package.h"

#include "miptp_send_job.h"
#include "miptp_receive_job.h"

// CONN_COMPLETE means the connection has done its job and a reponse can be sent to the client
// CONN_DONE means the connection can be removed
enum AppConnectionStatus {CONN_SENDER = 1, CONN_RECEIVER = 2, CONN_NOT_READY = 3, CONN_COMPLETE = 4, CONN_FINISHED = 5};

/*
    Represents a connection to a external application
    Only one of s_job and r_job attributes will be set and used depending on the type.
*/
typedef struct AppConnection
{
    uint16_t port: 14;
    int socket;
    enum AppConnectionStatus status;
    MIPTPSendJob *s_job;
    MIPTPReceiveJob *r_job;
} AppConnection;


AppConnection_create(enum AppConnectionStatus status,
                    int socket,
                    uint16_t port,
                    MIP_ADDRESS destination,
                    BYTE *data,
                    uint16_t data_size,
                    unsigned long timeout);

AppConnection_create_not_ready(int socket);

void AppConnection_destroy(AppConnection *connection);

void AppConnection_close(AppConnection *connection);

void AppConnection_close_destroy(AppConnection *connection);

Queue *AppConnection_next_packages(AppConnection *connection);

int AppConnection_receive_package(AppConnection *connection, MIPTPPackage *package);

// AppConnectionStatus must be complete else a NULL pointer will be returned
ClientPackage *AppConnection_result(AppConnection *connection);

#define AppConnection_status_is(A, S) ((A)->status == S)

#define AppConnection_is_complete(A) ((A)->status == CONN_COMPLETE)

#endif