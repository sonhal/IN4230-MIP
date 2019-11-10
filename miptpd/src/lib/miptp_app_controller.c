#include <sys/epoll.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>

#include "../../../commons/src/dbg.h"
#include "../../../commons/src/polling.h"

#include "miptp_app_controller.h"
#include "../../../commons/src/client_package.h"


AppConnection_create(uint16_t port, int socket){
    AppConnection *connection = calloc(1, sizeof(AppConnection));
    check_mem(connection);

    connection->port = port;
    connection->socket = socket;

    return connection;

    error:
        log_err("Failed to create AppConnection");
        return NULL;
}

void AppConnection_destroy(AppConnection *connection){
    if(connection){
        if(connection->socket) close(connection->socket);
        free(connection);
    }
}



MIPTPAppController *MIPTPAppController_create(int mipd_socket, unsigned int max_connections){
    MIPTPAppController *controller = calloc(1, sizeof(MIPTPAppController));
    check_mem(controller);
    controller->connections = List_create();
    controller->max_connections = max_connections;
    controller->mipd_socket = mipd_socket;

    return controller;

    error:
        log_err("Failed to create MIPTPAppController in MIPTPAppController_create");
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

    LIST_FOREACH(controller->connections, first, next, cur){
        AppConnection *connection = (AppConnection *)cur->value;
        check(connection != NULL, "Invalid value, connection is NULL");

        if(connection->socket == socket){
            connection->port = package->port;
            connection->type = package->type;
            connection->job = MIPTPJob_create(connection->port, package->data, package->data_size, controller->timeout);
            check(connection->job != NULL, "Failed to set the MIPTPJob");
        }
    }
    return 1;

    error:
        return -1;
}


int MIPTPAppController_handle_mipd_package(MIPTPAppController *app_controller, int mipd_socket, BYTE *s_package){
    check(app_controller != NULL, "Bad argument, app_controller is NULL");
    check(s_package != NULL, "Bad argument, s_package is NULL");
    MIPTPPackage *package = ClientPackage_deserialize(s_package);
    check(package != NULL, "Failed to deserialize MIPTPPackage from mipd");

    LIST_FOREACH(app_controller->connections, first, next, cur){
        AppConnection *connection = (AppConnection *)cur->value;
        check(connection != NULL, "Invalid value, connection is NULL");

        if(connection->port == package->miptp_header.port){
            check(connection->job != NULL, "Invalid state connection(port: %d) MIPTPJob is NULL", connection->port);
            MIPTPJob_receive_package(connection->job, package);
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
    AppConnection  *connection = AppConnection_create(0, new_socket);

    List_push(app_controller->connections ,connection);

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


int MIPTPAppController_pump(MIPTPAppController *controller){
    int rc = 0;
    Queue  *next_packages = NULL;

    LIST_FOREACH(controller->connections, first, next, cur){
        AppConnection *connection = (AppConnection *)cur->value;
        check(connection != NULL, "Invalid value, connection is NULL");

        // If a Job has been created for the connection
        if(connection->job != NULL){
            next_packages = MIPTPJob_next_packages(connection->job);
            QUEUE_FOREACH(next_packages, q_cur){
                // TODO - Push on the network
                MIPTPPackage *package = (MIPTPPackage *)q_cur->value;
                check(package != NULL, "Invalid state, MIPTPPackage is NULL");

                rc = MIPTPAppController_send(controller, controller->mipd_socket, package);
                check(rc != -1, "Failed to send package(seqnr: %d, port: %d) to mipd", package->miptp_header.PSN, package->miptp_header.port);
            }
            Queue_destroy(next_packages);
        }
    }

    return 1;

    error:
        return -1;
}


int MIPTPAppController_send(MIPTPAppController *controller, int socket, MIPTPPackage *package){
    check(controller != NULL, "controller argument is NULL");
    check(package != NULL, "package argument is NULL");
    int rc = 0;
    BYTE *s_package = calloc(sizeof(MIPTPPackage) + package->data_size, sizeof(BYTE));
    check_mem(s_package);

    rc = MIPTPPackage_serialize(s_package, package);
    check(rc != -1, "Bad seralization of the package");

    rc = send(socket, s_package, rc, 0);
    check(rc != -1, "Failed to send the package to mipd - socket: %d\tbytes: %d", socket, rc);

    MIPTPPackage_destroy(package);
    if(s_package) free(s_package);

    return 1;

    error:
        return -1;
}