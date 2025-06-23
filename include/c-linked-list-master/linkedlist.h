#ifndef LINKEDLIST_HEADER
#define LINKEDLIST_HEADER

#include <stdlib.h>
#include <stdio.h>

#ifndef LINKEDLIST_DATA_TYPE
#include <stdint.h>
#define LINKEDLIST_DATA_TYPE uint64_t
#define LINKEDLIST_DATA_TYPE_FORMAT "%llu\n"
#endif

typedef struct ll_node_t ll_Node;
typedef struct ll_list_t ll_List;

struct ll_node_t {
  LINKEDLIST_DATA_TYPE data;
  struct ll_node_t *next;
};

struct ll_list_t {
  ll_Node *head;
};

ll_List *ll_makelist();
void ll_add(LINKEDLIST_DATA_TYPE data, ll_List *list);
void ll_delete(LINKEDLIST_DATA_TYPE data, ll_List *list);
void ll_display(ll_List *list);
void ll_reverse(ll_List *list);
void ll_reverse_using_two_pointers(ll_List *list);
void ll_destroy(ll_List *list);

#ifdef LINKEDLIST_IMPLEMENTATION

ll_Node *ll_createnode(LINKEDLIST_DATA_TYPE data);
inline ll_Node *ll_createnode(LINKEDLIST_DATA_TYPE data){
  ll_Node *newNode = malloc(sizeof(ll_Node));
  if (!newNode) {
    return NULL;
  }
  newNode->data = data;
  newNode->next = NULL;
  return newNode;
}

inline ll_List *ll_makelist() {
  ll_List *list = malloc(sizeof(ll_List));
  if (!list) {
    return NULL;
  }
  list->head = NULL;
  return list;
}

inline void ll_display(ll_List *list) {
  ll_Node *current = list->head;
  if(list->head == NULL)
    return;

  for(; current != NULL; current = current->next) {
    printf(LINKEDLIST_DATA_TYPE_FORMAT, current->data);
  }
}

inline void ll_add(LINKEDLIST_DATA_TYPE data, ll_List *list){
  ll_Node *current = NULL;
  if(list->head == NULL){
    list->head = ll_createnode(data);
  }
  else {
    current = list->head;
    while (current->next!=NULL){
      current = current->next;
    }
    current->next = ll_createnode(data);
  }
}

inline void ll_delete(LINKEDLIST_DATA_TYPE data, ll_List *list){
  ll_Node *current = list->head;
  ll_Node *previous = current;
  while(current != NULL){
    if(current->data == data){
      previous->next = current->next;
      if(current == list->head)
        list->head = current->next;
      free(current);
      return;
    }
    previous = current;
    current = current->next;
  }
}

inline void ll_reverse(ll_List *list){
  ll_Node *reversed = NULL;
  ll_Node *current = list->head;
  ll_Node *temp = NULL;
  while(current != NULL){
    temp = current;
    current = current->next;
    temp->next = reversed;
    reversed = temp;
  }
  list->head = reversed;
}

//Reversing the entire list by changing the direction of link from forward to backward using two pointers
inline void ll_reverse_using_two_pointers(ll_List *list){
    ll_Node *previous = NULL;
    while (list->head)
    {
        ll_Node *next_node = list->head->next; //points to second node in list
        list->head->next = previous;//at initial making head as NULL
        previous = list->head;//changing the nextpointer direction as to point backward node
        list->head = next_node; //moving forward by next node
    }
    list->head=previous;
}

inline void ll_destroy(ll_List *list){
  ll_Node *current = list->head;
  ll_Node *next = current;
  while(current != NULL){
    next = current->next;
    free(current);
    current = next;
  }
  free(list);
}

#endif
#endif
