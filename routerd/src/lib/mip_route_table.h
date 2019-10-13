#ifndef _MIP_ROUTE_TABLE_H
#define _MIP_ROUTE_TABLE_H

#include <stdint.h>
#include "../../../commons/src/list.h"
#include "../../../commons/src/time_commons.h"


#define MIP_ADDRESS uint8_t
#define MIP_BROADCAST_ADDRESS 255
#define MIP_TABLE_PACKAGE_ENTRIES_MAX_SIZE 64
#define MIP_TABLE_ENTRY_MAX_AGE_MILLI 8000

typedef struct MIPRouteEntry {
    MIP_ADDRESS destination;
    MIP_ADDRESS next_hop;
    unsigned int cost;
    long last_updated_milli;
} MIPRouteEntry;

typedef struct MIPRouteTable {
    MIP_ADDRESS table_address;
    List *entries;
} MIPRouteTable;

typedef struct MIPRoutePackageEntry {
    MIP_ADDRESS destination;
    MIP_ADDRESS next_hop;
    unsigned int cost;
} MIPRoutePackageEntry;


struct MIPRouteTablePackage {
    MIP_ADDRESS table_address;
    uint8_t num_entries;
    MIPRoutePackageEntry entries[MIP_TABLE_PACKAGE_ENTRIES_MAX_SIZE];
}__attribute__((packed));

typedef struct MIPRouteTablePackage MIPRouteTablePackage;


MIPRouteTable *MIPRouteTable_create();

void MIPRouteTable_destroy(MIPRouteTable *table);

int MIPRouteTable_update(MIPRouteTable *table, MIP_ADDRESS destination, MIP_ADDRESS next_hop, int cost);

int MIPRouteTable_remove(MIPRouteTable *table, MIP_ADDRESS destination);

// Gets the next hop to the requested MIP_ADDRESS destination 
MIP_ADDRESS MIPRouteTable_get_next_hop(MIPRouteTable *table, MIP_ADDRESS destination);

MIPRouteTablePackage *MIPRouteTable_create_package(MIPRouteTable *table);

#define MIPRouteTablePackage_destroy(A) if((A)) free(A)

MIPRouteTable *MIPRouteTablePackage_create_table(MIPRouteTablePackage *package);

#define MIPRouteTable_count(T) (List_count((T)->entries))

// Returns the MIPRouteEntry for the MIP address if it is present, NULL if it is not
static inline MIPRouteEntry *MIPRouteTable_get(MIPRouteTable *table, MIP_ADDRESS destination){
    LIST_FOREACH(table->entries, first, next, cur){
        MIPRouteEntry *current = cur->value;
        if(current->destination == destination){
            return current;
        }
    }
    return NULL;
}

MIP_ADDRESS MIPRouteTable_get_next_hop(MIPRouteTable *table, MIP_ADDRESS destination);

void MIPRouteTable_update_routing(MIPRouteTable *table, MIPRouteTable *neighbor_table);

void MIPRouteTable_print(MIPRouteTable *table);

// Removes entries that are older than max age, retuns number of entries removed on success, -1 on failure
int MIPRouteTable_remove_old_entries(MIPRouteTable *table);

#endif