
#include <unistd.h>

#include "../../../commons/src/dbg.h"
#include "../../../commons/src/time_commons.h"

#include "routing_handler.h"
#include "router_server.h"

int broadcast_route_table(MIPRouteTable *table, int socket){
    int rc = 0;
    MIPRouteTablePackage *package = MIPRouteTable_create_package(table);
    check_mem(package);

    printf("sending route table packet with num entries %d\n", package->num_entries);
    rc = write(socket, package, sizeof(MIPRouteTablePackage));
    printf("sendt route table packet of size: %d\n", rc);
    check(rc != -1, "Failed to send forwarding response");

    MIPRouteTablePackage_destroy(package);
    return rc;

    error:
        MIPRouteTablePackage_destroy(package);
        return -1;
}

MIPRouteTable *handle_route_request(MIPRouteTable *table, int socket){
        int rc = 0;

    MIPRouteTablePackage *routing_package = calloc(1, sizeof(MIPRouteTablePackage));

    rc = recv(socket, routing_package, sizeof(MIPRouteTablePackage), 0);
    check(rc != -1, "Failed to recv routing package");

    MIPRouteTable *neighbor_table = MIPRouteTablePackage_create_table(routing_package);
    MIPRouteTablePackage_destroy(routing_package);

    return neighbor_table;

    error:
        MIPRouteTablePackage_destroy(routing_package);
        return NULL;
}

int should_complete_route_broadcast(RouterServer *server){
    long current_time = get_now_milli();
    long last_update = server->last_broadcast_milli;
    if(current_time - last_update > server->broadcast_freq_milli) return 1;
    return 0;
}
