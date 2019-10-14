#include <stdlib.h> 		/* malloc */
#include <stdio.h>		/* printf */
#include <string.h>		/* memset, strcmp, strncpy */
#include <sys/socket.h>		/* socket, bind, listen, accept */
#include <sys/un.h>		/* struct sockaddr_un */
#include <unistd.h>		/* read, close, unlink */
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>

#include "../../../commons/src/dbg.h"
#include "../../../commons/src/application.h"
#include "app_connection.h"

#include "packaging/mip_package.h"

#define BUF_SIZE 256


// Sets up a socket wit the given file name
int create_domain_socket(){
    int so = 0;

    so = socket(AF_UNIX, SOCK_STREAM, 0);
    check(so != -1, "Creating socket failed");
    debug("Domain socket created: %d", so);
    return so;

    error:
        if (so) close(so);
        return -1;
}

int create_domain_socket_of_type(enum __socket_type type){
    int so = 0;

    so = socket(AF_UNIX, type, 0);
    check(so != -1, "Creating socket failed");
    debug("Domain socket created: %d", so);
    return so;

    error:
        if (so) close(so);
        return -1;
}


void LocalSocket_destroy(LocalSocket *local_socket){
    if(local_socket){
        close(local_socket->listening_socket_fd);
        close(local_socket->connected_socket_fd);
        if(local_socket->so_name) {
            unlink(local_socket->so_name->sun_path);
            free(local_socket->so_name);
        }
        free(local_socket);
    }
}

LocalSocket *LocalSocket_create(char *path, enum __socket_type socket_type){
    int rc = 0;
    LocalSocket *local_socket = calloc(1, sizeof(LocalSocket));
    local_socket->so_name = calloc(1, sizeof(struct sockaddr_un));
    local_socket->connected_socket_fd = -1;
    size_t path_len = strnlen(path, 255);

    check(path_len <= sizeof(local_socket->so_name->sun_path), "path len is to large");

    local_socket->listening_socket_fd = create_domain_socket_of_type(socket_type);

    // Zero out the name struct
    memset(local_socket->so_name , 0, sizeof(struct sockaddr_un));

    // Prepare UNIX socket name
    local_socket->so_name ->sun_family = AF_UNIX;
    strncpy(local_socket->so_name->sun_path, path, sizeof(local_socket->so_name->sun_path) - 1);

    // Delete socket file if it already exists
    unlink(local_socket->so_name->sun_path);

    /* Bind socket to socket name (file path)
       What happes if we pass &so_name? */
    rc = bind(local_socket->listening_socket_fd, (const struct sockaddr*)local_socket->so_name, sizeof(struct sockaddr_un));
    check(rc != -1, "Binding socket to local address failed");

    if( socket_type == SOCK_STREAM){
        // Listen for connections
        rc = listen(local_socket->listening_socket_fd, 5);
        check(rc != -1, "Failed to start listen");
    }

    return local_socket;

    error:
        LocalSocket_destroy(local_socket);
        return NULL;
}


int setup_domain_socket(struct sockaddr_un *so_name, char *socket_name, unsigned int socket_name_size){
    int so = 0;
    int rc = 0;
    
    check(socket_name_size <= sizeof(so_name->sun_path), "Socket name is to large");

    so = create_domain_socket();

    // Zero out the name struct
    memset(so_name, 0, sizeof(struct sockaddr_un));

    // Prepare UNIX socket name
    so_name->sun_family = AF_UNIX;
    strncpy(so_name->sun_path, socket_name, sizeof(so_name->sun_path) - 1);

    // Delete socket file if it already exists
    unlink(so_name->sun_path);

    /* Bind socket to socket name (file path)
       What happes if we pass &so_name? */
    rc = bind(so, (const struct sockaddr*)so_name, sizeof(struct sockaddr_un));
    check(rc != -1, "Binding socket to local address failed");

    // Listen for connections
    rc = listen(so, 5);
    check(rc != -1, "Failed to start listen");

    return so;

    error:
        close(so);
        unlink(so_name->sun_path);
        return -1;
}


int parse_domain_socket_request(char *buffer, uint8_t *mip_addr, char *message){
    int i = 0;
    char *format = "%d %s";
    return sscanf(buffer, format, mip_addr, message);
}

struct ping_message *parse_ping_request(BYTE *buffer){
    struct ping_message *request = calloc(1, sizeof(struct ping_message));
    memcpy(request, buffer, sizeof(struct ping_message));
    return request;
}

// returns bytes read into buffer on success, -1 on failure and 0 if connection is closed.
int read_from_domain_socket(int socket_fd, BYTE *buffer, size_t buffer_size){
    int rc = 0;

    rc = read(socket_fd, buffer, buffer_size);
    check(rc != -1, "Failed to receive packet on domain socket: %d", socket_fd);

    return rc;

    error:
        return -1;
}

// Returns a empty MIPPackage with the provided ping_message to be added to queue
MIPPackage *create_queueable_ping_message_MIPPackage(struct ping_message *message){
    BYTE default_mac[] = {0, 0, 0, 0, 0, 0};
    MIPPackage *m_package = NULL;

    // Create MIP packet
    m_package = MIPPackage_create_raw(0, &default_mac, message->dst_mip_addr, &default_mac, message, sizeof(struct ping_message), 3);
    check(m_package != NULL, "Failed to create MIPPackage");

    return m_package;

    error:
        return NULL;
}