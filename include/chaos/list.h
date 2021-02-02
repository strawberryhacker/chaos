// Ciruclar doubly linked list implementation

#ifndef LIST_H
#define LIST_H

#include <chaos/types.h>
#include <stddef.h>

// This structure will represent both a list and a list node. This implies that the list 
// will be circular
struct list_node {
    struct list_node* next;
    struct list_node* prev;
};

// Returns a pointer to the struct entry in which the list is embedded
#define list_get_struct(node, type, member) \
    (type *)((u8 *)node - offsetof(type, member))

// Initializes a list
static inline void list_init(struct list_node* list) {
    list->prev = list;
    list->next = list;
}

static inline void list_node_init(struct list_node* list) {
    // TODO: NULL trap
    list->next = NULL;
    list->prev = NULL;
}

// Inserts a list node between two consecutive entries
static inline void __list_add(struct list_node* new, struct list_node* prev,
    struct list_node* next) {
    prev->next = new;
    next->prev = new;
    new->next = next;
    new->prev = prev;
}

// Delets a node from the list between two consecutive entries
static inline void __list_delete(struct list_node* prev, struct list_node* next) {
    prev->next = next;
    next->prev = prev;
}

// Inserts a list node first in the list
static inline void list_push_front(struct list_node* new, struct list_node* list) {
    __list_add(new, list, list->next);
}

// Inserts a list node last in the list
static inline void list_push_back(struct list_node* new, struct list_node* list) {
    __list_add(new, list->prev, list);
}

static inline void list_push_before(struct list_node* new, struct list_node* before) {
    __list_add(new, before->prev, before);
}

// Deletes a node in the list
static inline void list_pop(struct list_node* node) {
    __list_delete(node->prev, node->next);

    // TODO: add MPU protected address, issue #50 
    node->next = NULL;
    node->prev = NULL;
}

// Deletes the first node in the list
static inline struct list_node* list_pop_front(struct list_node* list) {
    if (list->next == list) {
        return NULL;
    }
    struct list_node* node = list->next;
    list_pop(list->next);
    return node;
}

// Deletes the last node in the list
static inline struct list_node* list_pop_back(struct list_node* list) {
    if (list->prev == list) {
        return NULL;
    }
    struct list_node* node = list->prev;
    list_pop(list->prev);
    return node;
}

static inline u8 list_is_empty(struct list_node* list) {
    return (list->next == list) ? 1 : 0;
}

#define list_get_first(list) ((list)->next)
#define list_get_last(list) ((list)->prev)

// Defines for iterating lists
#define list_iterate(node, list) \
    for (node = (list)->next; node != (list); node = node->next)

#define list_iterate_reverse(node, list) \
    for (node = (list)->prev; node != (list); node = node->prev)

static inline u32 list_get_size(struct list_node* list) {
    u32 size = 0;
    
    struct list_node* node;
    list_iterate(node, list) {
        size++;
    }
    return size;
}

// Merges list src into list dest
static inline void list_merge(struct list_node* src, struct list_node* dest) {
    src->prev->next = dest->next;
    dest->next->prev = src->prev;

    dest->next = src->next;
    dest->next->prev = dest;
}

#endif
