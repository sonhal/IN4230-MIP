#ifndef _COMMONS_LIST_H
#define _COMMONS_LIST_H

#include <stdlib.h>

struct ListNode;

typedef struct ListNode
{
    struct ListNode *next;
    struct ListNode *prev;
    void *value;
} ListNode;

typedef struct List {
    int count;
    ListNode *first;
    ListNode *last;
} List;

// Creates a empty List
List *List_create();

// Frees the List and ListNodes
void List_destroy(List *list);

// Frees all values io the List
void List_clear(List *list);

// Frees the List, ListNodes and values
void List_clear_destroy(List *list);


#define List_count(A) ((A)->count)
#define List_first(A) ((A)->first != NULL ? (A)->first->value : NULL)
#define List_last(A) ((A)->last != NULL ? (A)->last->value : NULL)

// Adds value to end of list
void List_push(List *list, void *value);

// Takes the last value of the List and returns it
void *List_pop(List *list);

// Adds a value to the front of the list
void List_unshift(List *list, void *value);

// Removes the first value in the List and returns it
void *List_shift(List *list);

// Removes a node in the list, O(N)
void *List_remove(List *list, ListNode *node);

#define LIST_FOREACH(L, S, M, V) ListNode *_node = NULL;\
                                ListNode *V = NULL;\
                                for(V = _node = L->S; _node != NULL; V = _node = _node->M)


#endif