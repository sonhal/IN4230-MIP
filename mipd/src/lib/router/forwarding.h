#ifndef _FORWARDING_H
#define _FORWARDING_H

#include "../../../../commons/src/application.h"
#include "../../../../commons/src/time_commons.h"
#include "../../../../commons/src/queue.h"

#include "../server.h"

#define MAX_FORWARD_QUEUE_ENTRY_AGE_MILLI 1000

ForwardQueue *ForwardQueue_create();

inline void ForwardQueue_destroy(ForwardQueue *fq);

// returns 1 if response was found, -1 if not
int forward_found(MIP_ADDRESS forward_response);

int request_forwarding(MIPDServer *server, MIP_ADDRESS destination, MIPPackage *package);

MIPPackage *ForwardQueue_pop(ForwardQueue *fq);

#endif