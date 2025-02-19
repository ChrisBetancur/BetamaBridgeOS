#ifndef LIST_H
#define LIST_H

#include <stddef.h>

#define DEFINE_LIST_STRUCTURE(type) \
typedef struct type##_list_node { \
    type* head; \
    type* tail; \
    uint32_t size; \
} type##_list;

#define INIT_LIST(list) \
    list.head = (void*)0; \
    list.tail=(void*)0; \
    list.size=0;

#define PAIR_LIST_NODE(type) \
    type* next; \
    type* prev;

#define DEFINE_LIST_FUNCTIONS(type) \
void append_##type##_list(type##_list* list, type* node) {\
    if (list->size == 0) { \
        list->head = node; \
        list->tail = node; \
        node->prev = NULL; \
        node->next = NULL; \
    } else { \
        list->head->prev = node; \
        node->next = list->head; \
        list->head = node; \
        node->prev = NULL; \
    } \
    list->size++; \
}\
void append_##type##_list_batch(type##_list* list, type** batch, uint32_t num_nodes) {\
    if (num_nodes == 0) { \
        return; \
    } \
    \
    for (uint32_t i = 0; i < num_nodes - 1; i++) { \
        batch[i]->next = batch[i + 1]; \
        batch[i+1]->prev = batch[i]; \
    } \
    batch[0]->prev = NULL; \
    batch[num_nodes]->next = NULL; \
    \
    if (list->size == 0) { \
        list->head = batch[0]; \
        list->tail = batch[num_nodes]; \
        list->size += num_nodes; \
    } else { \
        list->head->prev = batch[num_nodes]; \
        batch[num_nodes]->next = NULL; \
        list->head = batch[0]; \
        list->tail = batch[num_nodes]; \
        list->size += num_nodes; \
    } \
}\
\
void push_##type##_list(type##_list* list, type* node) {\
    list->head->prev = node; \
    node->next = list->head; \
    list->head = node; \
    node->prev = NULL; \
    list->size++; \
}\
\
type* pop_##type##_list(type##_list* list) {\
    if (list->size == 0) { \
        return NULL; \
    } \
    type* node = list->head; \
    list->head = list->head->next; \
    list->head->prev = NULL; \
    list->size--; \
    return node; \
}\


#endif