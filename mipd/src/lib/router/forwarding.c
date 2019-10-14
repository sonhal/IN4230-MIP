
#include "../../../../commons/src/dbg.h"

#include "forwarding.h"
#include "../packaging/mip_package.h"
#include "../link.h"
#include "../mip_arp.h"

ForwardQueue *ForwardQueue_create(){
    ForwardQueue *fq = calloc(1, sizeof(ForwardQueue));
    fq->queue = Queue_create();
    return fq;
}

void ForwardQueue_destroy(ForwardQueue *fq){
    if(fq){
        if(fq->queue) Queue_clear_destroy(fq->queue);
        free(fq);
    }
}

int ForwardQueue_push(ForwardQueue *fq, MIPPackage *package){
    ForwardQueueEntry *entry = calloc(1, sizeof(ForwardQueueEntry));
    entry->age_milli = get_now_milli();
    entry->destination = package->m_header.dst_addr;
    entry->package = package;
    Queue_send(fq->queue, entry);
}

MIPPackage *ForwardQueue_pop(ForwardQueue *fq){
    ForwardQueueEntry *entry = Queue_recv(fq->queue);
    check(entry != NULL, "Invalid return from queue, entry=NULL");
    MIPPackage *package = (MIPPackage *)entry->package;

    free(entry);
    check(package != NULL, "Invalid package stored in entry - package=NULL");
    return package;

    error:
        return NULL;
}

int forward_found(MIP_ADDRESS forward_response){
    if(forward_response == 255) return 0;
    return 1;
}


int load_forward_package(MIPDServer *server, MIPPackage *forward_package, MIP_ADDRESS next_hop){
    struct ether_frame *e_frame = NULL;
    struct mip_header *m_header = NULL;

    int sock = query_mip_address_src_socket(server->cache, next_hop);
    check(sock != -1, "could not locate mip address: %d in cache", next_hop);

    int i_pos = get_interface_pos_for_socket(server->i_table, sock);
    check(i_pos != -1, "Could not locate sock address=%d in interface table", sock);
    struct sockaddr_ll *sock_name = server->i_table->interfaces[i_pos].so_name;

    // set src mip address
    MIP_ADDRESS src_mip_addr = server->i_table->interfaces[i_pos].mip_address;

    int cache_pos = query_mip_address_pos(server->cache, next_hop);
    check(cache_pos != -1, "Could not locate cache pos");
       
    // Create headers for the message
    e_frame = create_ethernet_frame(server->cache->entries[cache_pos].dst_interface, sock_name);
    m_header = create_transport_mip_header(src_mip_addr, next_hop);

    // load MIP packet
    memcpy(&forward_package->e_frame, e_frame, sizeof(struct ether_frame));
    if(forward_package->m_header.src_addr == 255){
        forward_package->m_header.src_addr = src_mip_addr;
    }

    if(m_header) free(m_header);
    if(e_frame) free(e_frame);
    return 1;

    error:
        if(m_header) free(m_header);
        if(e_frame) free(e_frame);
        return -1;
}


int handle_forward_response(MIPDServer *server, MIP_ADDRESS next_hop){
    int rc = 0;    


    MIPPackage *forward_package = ForwardQueue_pop(server->forward_queue);
    rc = load_forward_package(server, forward_package, next_hop);
    check(rc != -1, "Failed to load forward package");

    int sock = query_mip_address_src_socket(server->cache, next_hop);
    check(sock != -1, "could not locate mip address: %d in cache", next_hop);

    int i_pos = get_interface_pos_for_socket(server->i_table, sock);
    check(i_pos != -1, "Could not locate sock address=%d in interface table", sock);

    // set src mip address
    MIP_ADDRESS src_mip_addr = server->i_table->interfaces[i_pos].mip_address;

    struct sockaddr_ll *sock_name = server->i_table->interfaces[i_pos].so_name;
    int cache_pos = query_mip_address_pos(server->cache, next_hop);
    check(cache_pos != -1, "Could not locate cache pos");

    // Send the message
    rc = sendto_raw_mip_package(sock, sock_name, forward_package);
    check(rc != -1, "Failed to send transport packet");
    MIPDServer_log(server, "Package forwarded to MIP address: %d", next_hop);

    return 1;

    error:
        return -1;
}

int request_forwarding(MIPDServer *server, MIP_ADDRESS destination, MIPPackage *package){
    int rc = 0;

    rc = write(server->forward_socket->connected_socket_fd, &destination, sizeof(MIP_ADDRESS));
    check(rc != -1, "Failed to send forward request(destionation=%d) to local routerd", destination);

    rc = ForwardQueue_push(server->forward_queue, package);
    check(rc != -1, "Failed to add forward request(destionation=%d) to forward queue", destination);

    return 1;

    error:
        return -1;
}