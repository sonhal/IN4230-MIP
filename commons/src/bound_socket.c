#include <stdlib.h> 		/* malloc */
#include <stdio.h>		/* printf */
#include <string.h>		/* memset, strcmp, strncpy */
#include <sys/socket.h>		/* socket, bind, listen, accept */
#include <sys/un.h>		/* struct sockaddr_un */
#include <unistd.h>		/* read, close, unlink */

#include "domain_socket.h"

#include "bound_socket.h"

BoundSocket *BoundSocket_create(char *file_path){
    int rc = 0;

    BoundSocket *b_socket = calloc(1, sizeof(BoundSocket));
    check_mem(b_socket);
    b_socket->so_name= calloc(1, sizeof(struct sockaddr_un));
    check_mem(b_socket->so_name);
    b_socket->file_path = calloc(strnlen(file_path, 256), sizeof(char));
    check_mem(b_socket->file_path);
    memcpy(b_socket->file_path, file_path, strnlen(file_path, 256));
    
    b_socket->socket_fd = -1;

    // Prepare UNIX socket name
    b_socket->so_name->sun_family = AF_UNIX;
    strncpy(b_socket->so_name->sun_path, file_path, sizeof(b_socket->so_name->sun_path) - 1);

    return b_socket;

    error:
        log_err("Failed to create BoundSocket");
        return NULL;
}

int BoundSocket_connect(BoundSocket *b_socket){
    int rc = 0;

    // Delete socket file if it already exists
    unlink(b_socket->so_name->sun_path);

    b_socket->socket_fd = create_domain_socket();
    check(b_socket->socket_fd != -1, "Failed to create domain socket");

    // Bind socket to socket name (file path)
    rc = bind(b_socket->socket_fd, (const struct sockaddr*)b_socket->so_name, sizeof(struct sockaddr_un));
    check(rc != -1, "Binding socket to local address failed");

    // Listen for connections
    rc = listen(b_socket->socket_fd, 5);
    check(rc != -1, "Failed to start listen");

    return 1;

    error:
        log_err("Failed to connect BoundSocket");
        return -1;
}

void BoundSocket_destroy(BoundSocket *b_socket){
    if(b_socket){
        if(b_socket->file_path) free(b_socket->file_path);
        if(b_socket->so_name){
            unlink(b_socket->so_name->sun_path);
            free(b_socket->so_name);
        } 
        close(b_socket->socket_fd);
        free(b_socket);
    }
}