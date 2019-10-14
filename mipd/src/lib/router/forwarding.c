
#include "../../../../commons/src/dbg.h"

#include "forwarding.h"
#include "../link.h"
#include "../server.h"
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

int ForwardQueue_push(ForwardQueue *fq, struct ping_message *p_message){
    ForwardQueueEntry *entry = calloc(1, sizeof(ForwardQueueEntry));
    entry->age_milli = get_now_milli();
    entry->destination = p_message->dst_mip_addr;
    entry->p_message = p_message;
    Queue_send(fq->queue, entry);
}

struct ping_message *ForwardQueue_pop(ForwardQueue *fq){
    ForwardQueueEntry *entry = Queue_recv(fq->queue);
    check(entry != NULL, "Invalid return from queue, entry=NULL");
    
    return entry->p_message;

    error:
        return NULL;
}

int forward_found(MIP_ADDRESS *forward_response){
    if(*forward_response == 255) return -1;
    return 1;
}


int handle_forward_response(MIPDServer *server, MIP_ADDRESS forward_response){
    int rc = 0;    
    struct mip_header *m_header = NULL;
    struct ether_frame *e_frame = NULL;
    MIPPackage *m_package = NULL;

    MIP_ADDRESS next_hop = forward_response;
    struct ping_message *p_message = ForwardQueue_pop(server->forward_queue);

    int sock = query_mip_address_src_socket(server->cache, next_hop);
    check(sock != -1, "could not locate mip address: %d in cache", p_message->dst_mip_addr);

    int i_pos = get_interface_pos_for_socket(server->i_table, sock);
    check(i_pos != -1, "Could not locate sock address=%d in interface table", sock);

    // set src mip address
    MIP_ADDRESS src_mip_addr = server->i_table->interfaces[i_pos].mip_address;
    p_message->src_mip_addr = src_mip_addr;

    struct sockaddr_ll *sock_name = server->i_table->interfaces[i_pos].so_name;
    int cache_pos = query_mip_address_pos(server->cache, p_message->dst_mip_addr);
    check(cache_pos != -1, "Could not locate cache pos");


    // Create headers for the message
    e_frame = create_ethernet_frame(server->cache->entries[cache_pos].dst_interface, sock_name);
    m_header = create_transport_mip_header(src_mip_addr, p_message->dst_mip_addr);

    // Create MIP packet
    p_message->src_mip_addr = m_header->src_addr; // Sett src address in ping message so it can be PONG'ed
    m_package = MIPPackage_create(e_frame, m_header, p_message, sizeof(p_message));

    // Send the message
    rc = sendto_raw_mip_package(sock, sock_name, m_package);
    check(rc != -1, "Failed to send transport packet");
    MIPDServer_log(server, "Forwarded Domain message sent");

    if(m_header) free(m_header);
    if(e_frame) free(e_frame);
    return 1;

    error:
        if(m_header) free(m_header);
        if(e_frame) free(e_frame);
        MIPPackage_destroy(m_package);
        return -1;
}

int request_forwarding(MIPDServer *server, struct ping_message *p_message){
    int rc = 0;

    rc = write(server->forward_socket->connected_socket_fd, &p_message->dst_mip_addr, sizeof(MIP_ADDRESS));
    check(rc != -1, "Failed to send forward request(destionation=%d) to local routerd", p_message->dst_mip_addr);

    rc = ForwardQueue_push(server->forward_queue, p_message);
    check(rc != -1, "Failed to add forward request(destionation=%d) to forward queue", p_message->dst_mip_addr);

    return 1;

    error:
        return -1;
}