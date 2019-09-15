#include <stdlib.h> 		/* malloc */
#include <stdio.h>		/* printf */
#include <string.h>		/* memset, strcmp, strncpy */
#include <sys/socket.h>		/* socket, bind, listen, accept */
#include <sys/un.h>		/* struct sockaddr_un */
#include <unistd.h>		/* read, close, unlink */
#include "dbg.h"


#define BUF_SIZE 256

void server(int l_so, struct sockaddr_un *so_name);

// Sets up a socket wit the given file name
int create_domain_socket(){
    int so = 0;

    so = socket(AF_UNIX, SOCK_STREAM, 0);
    check(so != -1, "Creating socket failed");

    return so;

    error:
        if (so) close(so);
        return -1;
}

int setup_domain_socket(struct sockaddr_un *so_name, char *socket_name, unsigned int socket_name_size){
    int so = 0;
    int rc = 0;
    
    check(socket_name_size <= sizeof(so_name->sun_path), "Socket name is to large");

    so = create_domain_socket();

    // Zero out the name struct
    memset(&so_name, 0, sizeof(struct sockaddr_un));

    // Prepare UNIX socket name
    so_name->sun_family = AF_UNIX;
    strncpy(so_name->sun_path, socket_name, sizeof(so_name->sun_path) - 1);

    // Delete socket file if it already exists
    unlink(so_name->sun_path);

    /* Bind socket to socket name (file path)
       What happes if we pass &so_name?*/
    rc = bind(so, (const struct sockaddr*)so_name, sizeof(struct sockaddr_un));
    check(rc != -1, "Binding socket to local address failed");

    return so;

    error:
        close(so);
        unlink(so_name->sun_path);
        return -1;
}


int app_server(int so, char *socket_name, unsigned int socket_name_size){
    struct sockaddr_un so_name;
    check(socket_name_size <= sizeof(so_name.sun_path), "Socket name is to large");

    // Zero out the name struct
    memset(&so_name, 0, sizeof(struct sockaddr_un));

    // Prepare UNIX socket name
    so_name.sun_family = AF_UNIX;
    strncpy(so_name.sun_path, socket_name, sizeof(so_name.sun_path) - 1);

    // Delete socket file if it already exisists
    unlink(so_name.sun_path);

    server(so, &so_name);

    unlink(so_name.sun_path);
    return 1;

    error:
        unlink(so_name.sun_path);
        return -1;
}

void server(int l_so, struct sockaddr_un *so_name) {
    int rc = 0;
    int so = 0;
    char *buf = calloc(1, BUF_SIZE);

    /* Bind socket to socket name (file path)
       What happes if we pass &so_name?*/
    rc = bind(l_so, (const struct sockaddr*)so_name, sizeof(struct sockaddr_un));
    check(rc != -1, "Binding socket to local address failed");

    // Listen for connections
    rc = listen(l_so, 5);
    check(rc != -1, "Failed to start listen");

    // Wait for connection
    while (1)
    {
        /* Block until we get a connection, then accept it,
           creating a new socket for that connection.
        */
       so = accept(l_so, NULL, NULL);
       check(so != -1, "Failed to accept connection");

       // We want to read at least once; do-while is useful for this
        do {
            rc = read(so, buf, BUF_SIZE - 1);

            if(rc > 0){
                printf("Received from socket: %d - size: %d\n[message]:%s\n", so, rc, buf);
                char return_message[128];
                sprintf(return_message, "[SERVER]: Received message of size: %d\n", rc);
                write(so, return_message, strlen(return_message));
            } else if (rc == 0){
                printf("Client using socket %d, went away\n", so);
            }else {
                sentinel("[SENTINEL] rc was %d", rc);
            }
            memset(buf, 0, BUF_SIZE);
        } while (rc > 0);
        close(so);
    }
    
    free(buf);
    return;

    error:
        if(so) close(so);
        free(buf);
        return;

}