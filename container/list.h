#pragma once

#include "container.h"

typedef struct list_node_t 
{
    any_t value;
    struct list_node_t * prev;
    struct list_node_t * next;
}list_node_t;

typedef struct 
{
    size_t size;
    list_node_t head;
    list_node_t tail;
}list_t;
typedef list_node_t * list_iter_t;


list_node_t * list_node_create();
list_t * list_create();
void list_node_destroy(list_node_t * node);
void list_init(list_t * list);
any_t list_front(list_t * list);
any_t list_back(list_t * list);
void list_push_front(list_t * list, any_t value);
void list_push_back(list_t * list, any_t value);
any_t list_pop_front(list_t * list);
any_t list_pop_back(list_t * list);
void list_destroy(list_t * list);
size_t list_size(list_t * list);
static inline list_iter_t list_begin(list_t * list) { return list->head.next; }
static inline list_iter_t list_end(list_t * list) { return &list->tail; }

static inline list_iter_t list_rbegin(list_t * list) { return list->tail.prev; }
static inline list_iter_t list_rend(list_t * list) { return &list->head; }
static inline any_t list_iter_get(list_iter_t iter) { return iter->value; }
static inline int list_empty(list_t * list) { return list_begin(list) == list_end(list); }
