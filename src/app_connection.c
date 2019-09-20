#include <stdlib.h> 		/* malloc */
#include <stdio.h>		/* printf */
#include <string.h>		/* memset, strcmp, strncpy */
#include <sys/socket.h>		/* socket, bind, listen, accept */
#include <sys/un.h>		/* struct sockaddr_un */
#include <unistd.h>		/* read, close, unlink */
#include "dbg.h"
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>



#define BUF_SIZE 256

void server(int l_so, struct sockaddr_un *so_name);

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