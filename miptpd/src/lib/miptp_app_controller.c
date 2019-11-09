#include <sys/epoll.h>

#include "../../../commons/src/dbg.h"
#include "../../../commons/src/polling.h"


#include "miptp_app_controller.h"
#include "../../../commons/src/client_package.h"

int MIPTPAppController_handle_package(MIPTPAppController *controller, int socket, BYTE *s_package){
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
            connection->job = MIPTPJob_create(package->data, package->data_size, controller->timeout);
            check(connection->job != NULL, "Failed to set the MIPTPJob");
        }
    } 
    return 1;

    error:
        return -1;
}


MIPTPAppController *MIPTPAppController_create(unsigned int max_connections){
    MIPTPAppController *controller = calloc(1, sizeof(MIPTPAppController));
    check_mem(controller);
    controller->connections = List_create();
    controller->max_connections = max_connections;

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
    LIST_FOREACH(controller->connections, first, next, cur){
        AppConnection *connection = (AppConnection *)cur->value;
        check(connection != NULL, "Invalid value, connection is NULL");

        // If a Job has been created for the connection
        if(connection->job != NULL){
            MIP
        }
    }

    error:
        return -1;
}