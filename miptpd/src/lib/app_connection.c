#include "app_connection.h"

AppConnection_create_not_ready(int socket){
    AppConnection *connection = AppConnection_create(CONN_NOT_READY, socket, -1, 0, NULL, 0, 0);
    check(connection != NULL, "Failed to create a NOT_READY AppConnection");

    return connection;

    error:
        return NULL;
}

enum AppConnectionStatus connection_type(enum MIPTPClientType type){
    if(type == MIPTP_SENDER) return CONN_SENDER;
    if(type == MIPTP_RECEIVER) return CONN_RECEIVER;

    log_err("Bad state - MIPTPClientType is invalid type: %d", type);
    return CONN_NOT_READY;
}

void AppConnection_destroy(AppConnection *connection){
    if(connection){
        if(connection->r_job) MIPTPReceiveJob_destroy(connection->r_job);
        if(connection->s_job) MIPTPSendJob_destroy(connection->s_job);
        free(connection);
    }
}

void AppConnection_close(AppConnection *connection){
    if(connection){
        if(connection->socket) close(connection->socket);
    }
}

void AppConnection_close_destroy(AppConnection *connection){
    AppConnection_close(connection);
    AppConnection_destroy(connection);
}


// Can return NULL if case of failure
AppConnection_create(enum AppConnectionStatus status,
                    int socket,
                    uint16_t port,
                    MIP_ADDRESS destination,
                    BYTE *data,
                    uint16_t data_size,
                    unsigned long timeout){

    AppConnection *connection = calloc(1, sizeof(AppConnection));
    check_mem(connection);

    connection->socket = socket;
    connection->port = port;
    connection->status = status;
    connection->r_job = NULL;
    connection->s_job = NULL;


    // Create and set the job for the connection
    switch (connection->status)
    {
    case CONN_SENDER:
        connection->s_job = MIPTPSendJob_create(connection->port, data, data_size, timeout);
        check(connection->s_job != NULL, "Failed to set the MIPTPSendJob");
        break;
    case CONN_RECEIVER:
        connection->r_job = MIPTPReceiveJob_create(connection->port, timeout);
        check(connection->s_job != NULL, "Failed to set the MIPTPReceiveJob");
        break;
    case CONN_NOT_READY:
        // Nothing
        break;
    case CONN_COMPLETE:
        // Nothing
        break;
    case CONN_FINISHED:
        // Nothing
        break;
    default:
        log_err("Bad state, connection type is in a invalid state: %d", connection->status);
        goto error;
    }

    return connection;

    error:
        return NULL;
}

Queue *AppConnection_next_packages(AppConnection *connection){
    check(connection != NULL, "Invalid argument, connection is NULL");

    Queue *next_packages = NULL;

    switch (connection->status)
    {
    case CONN_SENDER:
        next_packages = MIPTPSendJob_next_packages(connection->s_job);
        break;
    case CONN_RECEIVER:
        next_packages = MIPTPReceiveJob_next_packages(connection->r_job);
        break;
    default:
        next_packages = Queue_create();
        break;
    }
    check(next_packages != NULL, "Failed to create next_packages queue");
    
    return next_packages;

    error:
        return NULL;
}

int AppConnection_receive_package(AppConnection *connection, MIPTPPackage *package){
    
}

// AppConnectionStatus must be complete else a NULL pointer will be returned
ClientPackage *AppConnection_complete_response(AppConnection *connection);