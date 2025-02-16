#ifndef LIST_H
#define LIST_H

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
void append_##type##_list_node(type##_list* list, type* node) {\
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
\
void push_##type##_list_node(type##_list* list, type* node) {\
    list->head->prev = node; \
    node->next = list->head; \
    list->head = node; \
    node->prev = NULL; \
    list->size++; \
}\
\
type* pop_##type##_list_node(type##_list* list) {\
    if (list->size == 0) return (void*)0; \
    type* node = list->head; \
    list->head = list->head->next; \
    list->head->prev = NULL; \
    list->size--; \
    return node; \
}\

#endif