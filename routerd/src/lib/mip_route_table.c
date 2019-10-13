#include <stdint.h>
#include <stdlib.h>

#include "../../../commons/src/dbg.h"
#include "mip_route_table.h"


MIPRouteEntry *MIPRouteEntry_create(MIP_ADDRESS destination, MIP_ADDRESS next_hop, int cost){
    MIPRouteEntry *entry = calloc(1, sizeof(MIPRouteEntry));
    entry->cost = cost;
    entry->destination = destination;
    entry->next_hop = next_hop;
    entry->last_updated_milli = get_now_milli();
    return entry;
}

MIPRouteEntry *MIPRouteEntry_destroy(MIPRouteEntry *entry){
    if(entry) free(entry);
}

MIPRouteTable *MIPRouteTable_create(MIP_ADDRESS table_address){
    check(table_address < MIP_BROADCAST_ADDRESS, "Invalid address for a MIPRouteTable");

    MIPRouteTable *table = calloc(1, sizeof(MIPRouteTable));
    table->entries = calloc(1, sizeof(List));

    //Add route to itself
    MIPRouteTable_update(table, table_address, 255, 0);
    table->table_address = table_address;
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

// Updates the MIPRouteTable with a new route, removing the exsisiting route if it is present. Returns 0 if successfull
int MIPRouteTable_update(MIPRouteTable *table, MIP_ADDRESS destination, MIP_ADDRESS next_hop, int cost){
    MIPRouteEntry *entry = MIPRouteEntry_create(destination, next_hop, cost);

    // Remove entry if it already exists, does nothing if it doesnt
    MIPRouteTable_remove(table, entry->destination);
    List_push(table->entries, entry);

    return 0;
}

// Removes entry with destination from table, returns 1 if successful, -1 if entry could not be found
int MIPRouteTable_remove(MIPRouteTable *table, MIP_ADDRESS destination) {

        // Table is empty
        if(table->entries->count == 0) return 1;

        LIST_FOREACH(table->entries, first, next, cur){
        MIPRouteEntry *current = cur->value;
        check(current != NULL, "Invalid value from table, current is NULL");

        if(current->destination == destination){
            List_remove(table->entries, cur);
            return 1;
        }
    }

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

        // New never before seen node, add to table
        if(champion == NULL || challenger->cost + 1 < champion->cost){
            MIPRouteTable_update(table, challenger->destination, neighbor_table->table_address, challenger->cost + 1);
        }
    }

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
    package->table_address = table->table_address;
    
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

    MIPRouteTable *table = MIPRouteTable_create(package->table_address);
    for (int i = 0; i < package->num_entries; i++)
    {
        MIPRoutePackageEntry current =  package->entries[i];
        MIPRouteTable_update(table, current.destination, current.next_hop, current.cost);
    }

    return table;
    
    error:
        return NULL;
}
