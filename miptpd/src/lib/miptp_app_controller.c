#include <sys/epoll.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>

#include "../../../commons/src/dbg.h"
#include "../../../commons/src/polling.h"
#include "../../../commons/src/client_package.h"
#include "../../../commons/src/mipd_message.h"

#include "app_connection.h"
#include "miptp_app_controller.h"

MIPTPAppController *MIPTPAppController_create(int mipd_socket, unsigned int max_connections){
    MIPTPAppController *controller = calloc(1, sizeof(MIPTPAppController));
    check_mem(controller);
    controller->connections = List_create();
    controller->max_connections = max_connections;
    controller->mipd_socket = mipd_socket;

    return controller;

    error:
        log_err("Failed to create MIPTPAppController in MIPTPAppController_create");
        MIPTPAppController_destroy(controller);
        return NULL;
}


void MIPTPAppController_destroy(MIPTPAppController *app_controller){
    if(app_controller){
        if(app_controller->connections){
            List_destroy(app_controller->connections);
        }
        free(app_controller);
        app_controller = NULL;
    }
}


int MIPTPAppController_handle_app_package(MIPTPAppController *controller, int socket, BYTE *s_package){
    ClientPackage *package = ClientPackage_deserialize(s_package);
    check(package != NULL, "Failed to deserialize package from application");
    check(package->port > 0, "Invalid port");
    check(package->destination > 0, "Invalid destination");
    check(package->data != NULL, "Invalid data pointer, NULL");

    AppConnection *complete_connection = NULL;

    LIST_FOREACH(controller->connections, first, next, cur){
        AppConnection *not_ready_connection = (AppConnection *)cur->value;
        check(not_ready_connection != NULL, "Invalid value, not_ready_connection is NULL");

        if(not_ready_connection->socket == socket){
            enum AppConnectionStatus status = connection_type(package->type);
            complete_connection = AppConnection_create(status,
                                                            socket,
                                                            package->port,
                                                            package->destination,
                                                            package->data,
                                                            package->data_size,
                                                            controller->timeout);
            check(complete_connection != NULL, "Failed to create the complete AppConnection");

            // Remove and destroy the tmp connection
            List_remove(controller->connections, cur);
            AppConnection_destroy(not_ready_connection);
            List_push(controller->connections, complete_connection);
            break;
        }
    }

    return 1;

    error:
        return -1;
}



int MIPTPAppController_handle_mipd_package(MIPTPAppController *app_controller, int mipd_socket, MIPDMessage *message){
    check(app_controller != NULL, "Bad argument, app_controller is NULL");
    check(message != NULL, "Bad argument, MIPDMessage message is NULL");
    int rc = 0;
    MIPTPPackage *package = NULL;
    package = MIPTPPackage_deserialize(message->data, message->data_size);
    check(package != NULL, "Failed to deserialize MIPTPPackage");

    LIST_FOREACH(app_controller->connections, first, next, cur){
        AppConnection *connection = (AppConnection *)cur->value;
        check(connection != NULL, "Invalid value, connection is NULL");

        if(connection->port == package->miptp_header.port){

            rc = AppConnection_receive_package(connection, message->mip_address, package);
            check(rc != -1, "AppConnection failed to receive package");
        }
    } 
    return 1;

    error:
        return -1;
}


// Accepts a connection attempt from a application and registers the application.
// Returns file descriptor for the accepted socket on success, -1 on failure
int MIPTPAppController_handle_connection(MIPTPAppController *app_controller, int epoll_fd, struct epoll_event *event) {
    int rc = 0;
    int new_socket = 0;

    new_socket = accept(event->data.fd, NULL, NULL);
    check(new_socket != -1, "Failed to accept new application connection");

    struct epoll_event conn_event = create_epoll_in_event(new_socket);
    rc = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &conn_event);
    check(rc != -1, "Failed to add file descriptor to epoll");

    // TODO parse domain socket
    AppConnection  *connection = AppConnection_create_not_ready(new_socket);
    check(connection != NULL, "Error - new AppConnection is NULL");

    List_push(app_controller->connections, connection);

    return new_socket;

    error:
        log_warn("Failed to handle domain socket connection");
        return -1;
}


// Checks if a socket is used by a application connection
// Returns 1 if the socket is used by a connection
// Returns 0 if the socket is not used by a connection
// Returns -1 if a error occurred durring the lookup
int MIPTPAppController_is_a_connection(MIPTPAppController *controller, int socket){
    check(socket >= 0, "Invalid argument, socket is not valid: %d", socket);

    LIST_FOREACH(controller->connections, first, next, cur){
        AppConnection *connection = (AppConnection *)cur->value;
        check(connection != NULL, "Invalid value, connection is NULL");

        if(connection->socket == socket) return 1;
    }
    return 0;

    error:
        return -1;
}


// Returns 1 if successfully disconnected, 0 if not, -1 in the event of an error
int MIPTPAppController_disconnect(MIPTPAppController *controller, int socket){
    LIST_FOREACH(controller->connections, first, next, cur){
        AppConnection *connection = (AppConnection *)cur->value;
        check(connection != NULL, "Invalid value, connection is NULL");

        if(connection->socket == socket) {
            List_remove(controller->connections, cur);
            AppConnection_destroy(connection);
            return 1;
        }
    }
    return 0;

    error:
        log_err("Bad state in MIPTPAppController");
        return -1;
}


int MIPTPAppController_handle_outgoing(MIPTPAppController *controller){
    int rc = 0;
    Queue  *next_packages = NULL;

    LIST_FOREACH(controller->connections, first, next, cur){
        AppConnection *connection = (AppConnection *)cur->value;
        check(connection != NULL, "Invalid value, connection is NULL");

        // Will might return an empty queue
        next_packages = AppConnection_next_packages(connection);
        check(next_packages != NULL, "AppConnection failed to create next_packages");

        QUEUE_FOREACH(next_packages, q_cur){
            // TODO - Push on the network
            MIPTPPackage *package = (MIPTPPackage *)q_cur->value;
            check(package != NULL, "Invalid state, MIPTPPackage is NULL");
            
            BYTE p_buffer[MAX_MIPTP_PACKAGE_SIZE];
            rc = MIPTPPackage_serialize(&p_buffer, package);
            check(rc != -1, "Failed to serialise MIPTPPackage package");

            MIPDMessage *message = MIPDMessage_create(connection->connected_mip, rc, &p_buffer);
            check(message != NULL, "Failed to create MIPDMessage message");

            rc = MIPTPAppController_send_MIPDMessage(controller, controller->mipd_socket, message);
            check(rc != -1, "Failed to send package(seqnr: %d, port: %d) to mipd on sock: %d", package->miptp_header.PSN, package->miptp_header.port, controller->mipd_socket);
            MIPTPPackage_destroy(package);
        }
        Queue_destroy(next_packages);
    }

    return 1;

    error:
        return -1;
}

// Checks for finished packages returning the results to the connected applications
int MIPTPAppController_handle_completes(MIPTPAppController *controller){
    check(controller != NULL, "Invalid argument, controller is NULL");
    int rc = 0;
    ClientPackage *result = NULL;

    LIST_FOREACH(controller->connections, first, next, cur){
        AppConnection *connection = (AppConnection *)cur->value;
        check(connection != NULL, "Invalid value, connection is NULL");

        if(AppConnection_is_complete(connection)){
            result = AppConnection_result(connection);
            MIPTPAppController_send_ClientPackage(controller, connection->socket, result);

            AppConnection_close_destroy(connection);
            List_remove(controller->connections, cur);
        }
    }

    return 1;

    error:
        return -1;
}


int MIPTPAppController_send_MIPDMessage(MIPTPAppController *controller, int socket, MIPDMessage *message){
    BYTE m_buffer[MAX_MIPMESSAGE_SIZE];
    int rc = 0;
    uint16_t message_size = 0;
    check(controller != NULL, "controller argument is NULL");
    check(message != NULL, "package argument is NULL");
    

    message_size = MIPDMessage_serialize(&m_buffer, message);
    check(message_size != -1, "Bad seralization of the package");

    rc = send(socket, &m_buffer, message_size, 0);
    check(rc != -1, "Failed to send the package to mipd - socket: %d\tbytes: %d", socket, message_size);
    printf("sent package to mipd\tmip dest: %d\tmessage_size: %d\n",message->mip_address, message_size);

    MIPDMessage_destroy(message);
    return 1;

    error:
        return -1;
}


int MIPTPAppController_send_ClientPackage(MIPTPAppController *controller, int socket, ClientPackage *package){
    check(controller != NULL, "controller argument is NULL");
    check(package != NULL, "package argument is NULL");
    int rc = 0;
    BYTE *s_package = calloc(sizeof(ClientPackage) + package->data_size, sizeof(BYTE));
    check_mem(s_package);

    rc = ClientPackage_serialize(s_package, package);
    check(rc != -1, "Bad seralization of the package");

    rc = send(socket, s_package, rc, 0);
    check(rc != -1, "Failed to send the package to mipd - socket: %d\tbytes: %d", socket, rc);

    ClientPackage_destroy(package);
    if(s_package) free(s_package);

    return 1;

    error:
        return -1;
}