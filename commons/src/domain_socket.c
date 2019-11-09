
#include "dbg.h"
#include "domain_socket.h"

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

    // Bind socket to socket name (file path)
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


int connect_to_domain_socket(char *socket_file){
    int rc = 0;
    struct sockaddr_un so_name;
    int socket_fd = create_domain_socket();
    check(socket_fd != -1, "Failed to create domain socket");
    
    /* Prepare UNIX socket name */
    so_name.sun_family = AF_UNIX;
    strncpy(so_name.sun_path, socket_file, sizeof(so_name.sun_path) - 1);

    rc = connect(socket_fd, (const struct sockaddr*)&so_name, sizeof(struct sockaddr_un));
    check(rc != -1, "Failed to connect to domain socket: %s", so_name.sun_path);

    return socket_fd;

    error:
        return rc;
}