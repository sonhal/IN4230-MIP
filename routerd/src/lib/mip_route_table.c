#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

#include "../../../commons/src/dbg.h"
#include "mip_route_table.h"


MIPRouteEntry *MIPRouteEntry_create(MIP_ADDRESS destination, MIP_ADDRESS next_hop, unsigned int cost){
    check(destination <= MIP_BROADCAST_ADDRESS, "Invalid address for a MIPRouteTable");

    MIPRouteEntry *entry = calloc(1, sizeof(MIPRouteEntry));
    entry->cost = cost;
    entry->destination = destination;
    entry->next_hop = next_hop;
    entry->last_updated_milli = get_now_milli();
    return entry;

    error:
        return NULL;
}

MIPRouteEntry *MIPRouteEntry_destroy(MIPRouteEntry *entry){
    if(entry) free(entry);
}

MIPRouteTable *MIPRouteTable_create(){

    MIPRouteTable *table = calloc(1, sizeof(MIPRouteTable));
    check_mem(table);
    table->entries = List_create();
    check_mem(table->entries);
    table->table_address = 0;
    table->entry_max_age_milli = MIP_TABLE_ENTRY_MAX_AGE_MILLI;

    return table;

    error:
        return NULL;
}

void MIPRouteTable_destroy(MIPRouteTable *table){
    if(table){
        if(table->entries){
            List_clear_destroy(table->entries);
        }
        free(table);
    }
}

// Updates the MIPRouteTable with a new route, removing the exsisiting route if it is present. Returns 1 if successfull, -1 if it failed
int MIPRouteTable_update(MIPRouteTable *table, MIP_ADDRESS destination, MIP_ADDRESS next_hop, int cost){
    int rc = 0;
    MIPRouteEntry *entry = MIPRouteEntry_create(destination, next_hop, cost);
    check(entry != NULL, "Failed to create entry");

    // Remove entry if it already exists, does nothing if it doesnt
    rc = MIPRouteTable_remove(table, destination);
    check(rc != -1, "Failed to remove old destination: mip address: %d", destination);
    List_push(table->entries, entry);

    return 1;

    error:
        return -1;
}


// Removes entry with destination from table, returns 1 if successful, 0 if entry could not be found, -1 if it fails
int MIPRouteTable_remove(MIPRouteTable *table, MIP_ADDRESS destination) {

    // Table is empty
    if(table->entries->count == 0) return 1;

    LIST_FOREACH(table->entries, first, next, cur){
        MIPRouteEntry *current = (MIPRouteEntry *)cur->value;
        check(current != NULL, "Invalid value from table, current is NULL");

        if(current->destination == destination){
            MIPRouteEntry *removed_entry = List_remove(table->entries, cur);
            MIPRouteEntry_destroy(removed_entry);
            return 1;
        }
    }
    return 0;

    // Fall trough
    error:
        return -1;
}

// Gets the next hop to the requested MIP_ADDRESS destination 
MIP_ADDRESS MIPRouteTable_get_next_hop(MIPRouteTable *table, MIP_ADDRESS destination){
    check(table != NULL, "table is invalid, NULL");
    check(destination != MIP_BROADCAST_ADDRESS, "Lookup for broadcast address requested");

    MIPRouteEntry *entry = MIPRouteTable_get(table, destination);
    return entry != NULL ? entry->next_hop : 255;

    error:
        return MIP_BROADCAST_ADDRESS;
}

// Updates the Routing table with any new or better routes from the neighbor table
void MIPRouteTable_update_routing(MIPRouteTable *table, MIPRouteTable *neighbor_table){

    LIST_FOREACH(neighbor_table->entries, first, next, cur){
        MIPRouteEntry *challenger = cur->value;
        check(challenger != NULL, "Invalid value form neighbor table, NULL");

        MIPRouteEntry *champion = MIPRouteTable_get(table, challenger->destination);

        // New never before seen node, add to table. If the node has cost of unit max it is a poisoned reverse
        if((champion == NULL || (challenger->cost + 1) <= champion->cost) && challenger->cost != UINT_MAX){
            MIPRouteTable_update(table, challenger->destination, neighbor_table->table_address, (challenger->cost + 1));
        }
    }

    // Fall trough
    error:
        return;
}

void MIPRouteTable_print(MIPRouteTable *table){
    printf("-------------------------- MIP ROUTE TABLE --------------------------------\n");

    LIST_FOREACH(table->entries, first, next, cur){
        MIPRouteEntry *entry = cur->value;
        check(entry != NULL, "Entry from table is NULL");
        printf("Route: dst mip: %d\tnext hop: %d\tcost: %d\tlast updated: %ld\n",
            entry->destination,
            entry->next_hop,
            entry->cost,
            entry->last_updated_milli);
    }

    printf("---------------------------------------------------------------------------\n");

    error:
        return;
}

MIPRouteTablePackage *MIPRouteTable_create_package(MIPRouteTable *table){
    check(table->entries->count < MIP_TABLE_PACKAGE_ENTRIES_MAX_SIZE, "MIP Routing table is to large");
    MIPRouteTablePackage *package = calloc(1, sizeof(MIPRouteTablePackage));
    package->table_address = 0;
    
    int i = 0;
    LIST_FOREACH(table->entries, first, next, cur){
        MIPRouteEntry *current = cur->value;

        package->entries[i].destination = current->destination;
        package->entries[i].next_hop = current->next_hop;
        package->entries[i].cost = current->cost;
        i++;
    }
    package->num_entries = i;

    return package;

    error:
        return NULL;
}

MIPRouteTable *MIPRouteTablePackage_create_table(MIPRouteTablePackage *package){
    printf("received table package\tnum entries: %d\tsrc mip: %d\n",package->num_entries, package->table_address);
    check(package->num_entries < MIP_TABLE_PACKAGE_ENTRIES_MAX_SIZE, "MIP Routing table package is to large");

    MIPRouteTable *table = MIPRouteTable_create();
    table->table_address = package->table_address;

    for (int i = 0; i < package->num_entries; i++)
    {
        MIPRoutePackageEntry current =  package->entries[i];
        MIPRouteEntry *entry = MIPRouteEntry_create(current.destination, current.next_hop, current.cost);
        List_push(table->entries, entry);
    }

    return table;
    
    error:
        return NULL;
}

// Return 1 if entry is to old, 0 if it is not
int MIPRouteEntry_to_old(MIPRouteTable *table, MIPRouteEntry *entry){
    long current_time = get_now_milli();
    long last_update = entry->last_updated_milli;;
    if((current_time - last_update) > table->entry_max_age_milli) return 1;
    return 0;
}


int MIPRouteTable_remove_old_entries(MIPRouteTable *table){
    int rc = 0;
    int num_removed = 0;

    int i = 0;
    List *new_list = List_create();

    LIST_FOREACH(table->entries, first, next, cur){      
        MIPRouteEntry *entry = cur->value;
        check(entry != NULL, "Invalid entry - entry=NULL");

        if(MIPRouteEntry_to_old(table, entry) && entry->cost != 0){
            MIPRouteEntry_destroy(entry);
            num_removed++;
        } else {
            List_push(new_list, entry);
        }
        i++;
    }
    List_destroy(table->entries);
    table->entries = new_list;

    return num_removed;

    error:
        return -1;
}