#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>

#include "../../../commons/src/dbg.h"
#include "../../../commons/src/domain_socket.h"
#include "../../../commons/src/polling.h"

#include "miptp_app_controller.h"
#include "miptp_server.h"


#define PACKET_BUFFER_SIZE 65535
#define EVENTS_BUFFER_SIZE 5
#define POLLING_TIMEOUT 1000
#define MAX_APP_CONNECTIONS 10

MIPTPServer *MIPTPServer_create(char *mipd_socket, char *app_connections_socket, unsigned short int debug_active, unsigned long timeout) {
    MIPTPServer *server = calloc(1, sizeof(MIPTPServer));
    check_mem(server);
    check(strnlen(mipd_socket, 2) > 0, "Bad argument, mipd_socket is of length 0");
    check(strnlen(app_connections_socket, 2) > 0, "Bad argument, app_connections_socket is of length 0");
    
    server->app_connections_socket = BoundSocket_create(app_connections_socket);
    check(server->app_connections_socket != NULL, "Failed to set the BoundSocket");
    server->app_controller = MIPTPAppController_create(-1, MAX_APP_CONNECTIONS);
    check(server->app_controller != NULL, "Failed to set the AppController");

    server->debug_active = debug_active;
    server->timeout = timeout;

    server->mipd_socket = calloc(strnlen(mipd_socket, 256), sizeof(char));
    check_mem(server->mipd_socket);
    memcpy(server->mipd_socket, mipd_socket, strnlen(mipd_socket, 256));

    return server;

    error:
        log_err("Failed to create MIPTPServer");
        return NULL;
}

void MIPTPServer_destroy(MIPTPServer *server){
    if(server){
        if(server->mipd_socket) free(server->mipd_socket);
        if(server->app_connections_socket) BoundSocket_destroy(server->app_connections_socket);
        if(server->epoll_fd) close(server->epoll_fd);
        free(server);
    }
}

int MIPTPServer_init(MIPTPServer *server){
    int rc = 0;
    server->mipd_fd = connect_to_domain_socket(server->mipd_socket);
    check(server->mipd_fd != -1, "Failed to connect to mipd domain socket");

    rc = BoundSocket_connect(server->app_connections_socket);
    check(rc != -1, "Failed to connect the app connections socket");

    // Setup epoll
    struct epoll_event stdin_event = create_epoll_in_event(0); // listen to stdin for stop message
    struct epoll_event mipd_event = create_epoll_in_event(server->mipd_fd);
    struct epoll_event listen_event = create_epoll_in_event(server->app_connections_socket->socket_fd);
    struct epoll_event events_to_handle[] = {stdin_event, mipd_event, listen_event};

    server->epoll_fd = setup_epoll(&events_to_handle, 3);
    check(server->epoll_fd != -1, "Failed to setup epoll");

    // Set the correct mipd socket in the AppController
    server->app_controller->mipd_socket = server->mipd_socket;

    MIPTPServer_log(server, "MIPTPServer is initialized in debug mode");

    return 1;

    error:
        return -1;
}

int MIPTPServer_run(MIPTPServer *server){
    int rc = 0;
    int running = 1;
    int event_count = 0;
    size_t bytes_read = 0;
    BYTE read_buffer[PACKET_BUFFER_SIZE];
    struct epoll_event events[EVENTS_BUFFER_SIZE];

    while(running){
        MIPTPServer_log(server, " ...");

        event_count = epoll_wait(server->epoll_fd, &events, EVENTS_BUFFER_SIZE, POLLING_TIMEOUT);

        // Cycle the running jobs
        rc = MIPTPAppController_pump(server->app_controller);
        check(rc != -1, "Failed to cycle app controller jobs");

        int i = 0;
        for(i = 0; i < event_count; i++){
            memset(read_buffer, '\0', PACKET_BUFFER_SIZE);

            if(events[i].data.fd == server->mipd_fd){
                MIPTPServer_log(server, "mipd event");
                bytes_read = read(events[i].data.fd, read_buffer, PACKET_BUFFER_SIZE);
                if(bytes_read == 0){
                    MIPTPServer_log(server, "MIPD has disconnected, shutting down");
                    return 1;
                }else {           
                    rc = MIPTPAppController_handle_mipd_package(server->app_controller, events[i].data.fd, &read_buffer);
                    check(rc != -1, "Failed to handle package from mipd");
                }
                continue;
            }

            if(events[i].data.fd == server->app_connections_socket->socket_fd){
                MIPTPServer_log(server, "Connection event");
                rc = MIPTPAppController_handle_connection(server->app_controller, server->epoll_fd, &events[i]);
                check(rc != -1, "Error, failed to handle new application connection");
                continue;
            }

            if(MIPTPAppController_is_a_connection(server->app_controller, events[i].data.fd)){
                MIPTPServer_log(server, "Application event");
                bytes_read = read(events[i].data.fd, read_buffer, PACKET_BUFFER_SIZE);
                if(bytes_read == 0){
                    // Disconnect event
                    rc = MIPTPAppController_disconnect(server->app_controller, events[i].data.fd);
                    check(rc != -1, "Error, failed to disconnect AppConnection");
                    MIPTPServer_log(server, "Application disconnect");
                }else {           
                    rc = MIPTPAppController_handle_package(server->app_controller, events[i].data.fd, &read_buffer);
                    check(rc != -1, "Failed to handle package from application");
                }
                continue;
            }

            if(events[i].data.fd == 0){
                MIPTPServer_log(server, "stdin event");
                bytes_read = read(events[i].data.fd, read_buffer, PACKET_BUFFER_SIZE);

                if(!strncmp(read_buffer, "stop\n", 5)){
                    running = 0;
                    printf("Exiting...\n");
                } else {
                    printf("Unknown command\n");
                }
                continue;
            }
 
        }
    }

    return 1;

    error:
        return -1;
}