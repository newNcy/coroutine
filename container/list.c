#include "list.h"
#include <stdlib.h>

void list_node_init(list_node_t * node)
{
    node->value = nullptr;
    node->prev = nullptr;
    node->next = nullptr;
}


list_node_t * list_node_create()
{
    list_node_t * node = (list_node_t*)malloc(sizeof(list_node_t));
    list_node_init(node);
    return node;
}

void list_node_destroy(list_node_t * node)
{
    node->prev = nullptr;
    node->next = nullptr;
    node->value = nullptr;
    free(node);
}


void list_init(list_t * list)
{
    list->size = 0;
    list_node_init(&list->head);
    list_node_init(&list->tail);
    list->head.next = &list->tail;
    list->tail.prev = &list->head;
}

list_t * list_create()
{
    list_t * list = (list_t*)malloc(sizeof(list_t));
    list_init(list);
    return list;
}

any_t list_front(list_t * list)
{
    return list_begin(list)->value;
}

any_t list_back(list_t * list)
{
    return list_rbegin(list)->value;
}

void list_push_front(list_t * list, any_t value)
{
    list_node_t * node = list_node_create();
    list->size ++;
    node->value = value;

    list->head.next->prev = node;
    node->next = list->head.next;

    list->head.next = node;
    node->prev = &list->head;
}

void list_push_back(list_t * list, any_t value)
{
    list_node_t * node = list_node_create();
    node->value = value;
    list->size ++;

    list->tail.prev->next = node;
    node->prev = list->tail.prev;

    list->tail.prev = node;
    node->next = &list->tail;
}

any_t list_pop_front(list_t * list)
{
    list_node_t * n = list->head.next;
    list->head.next = n->next;
    n->next->prev = &list->head;

    any_t value = n->value;
    list_node_destroy(n);
    list->size --;

    return value;
}

any_t list_pop_back(list_t * list)
{
    list_node_t * n = list->tail.prev;

    list->tail.prev = n->prev;
    n->prev->next = &list->tail;

    any_t value = n->value;
    list_node_destroy(n);
    list->size --;

    return value;
}


void list_destroy(list_t * list)
{
    while (!list_empty(list)) {
        list_pop_front(list);
    }
    free(list);
}

size_t list_size(list_t * list)
{
    return list->size;
}
