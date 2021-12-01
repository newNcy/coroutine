#include "list.h"
#include <stdlib.h>

list_node_t * list_node_create()
{
    list_node_t * node = (list_node_t*)malloc(sizeof(list_node_t));
    node->value = nullptr;
    node->prev = nullptr;
    node->next = nullptr;
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
    list->head = nullptr;
    list->tail = nullptr;
}

any_t list_front(list_t * list)
{
    return list->head->value;
}

any_t list_back(list_t * list)
{
    return list->tail->value;
}

void list_push_front(list_t * list, any_t value)
{
    list_node_t * node = list_node_create();
    node->next = list->head;
    list->head->prev = node;

    node->value = value;
    list->head = node;
    list->size ++;
}

void list_push_back(list_t * list, any_t value)
{
    list_node_t * node = list_node_create();
    node->value = value;
    list->tail->next = node;
    node->prev = list->tail;
    node->value = value;
    list->size ++;
}

void list_pop_front(list_t * list)
{
    list_node_t * head = list->head->next;
    list_node_destroy(list->head);
    
    if (head) {
        head->prev = nullptr;
    } else {
        list->tail = nullptr;
    }
    list->head = head;
    list->size --;
}

void list_pop_back(list_t * list)
{
    list_node_t * tail = list->tail->prev;
    list_node_destroy(list->tail);

    if (tail) {
        tail->next = nullptr;
    } else {
        list->head = nullptr;
    }
    list->size --;
}


void list_destroy(list_t * list)
{
    list_node_t * node = list->head;
    while (node) {
        list_node_t * next = node->next;
        list_node_destroy(node);
        node = next;
    }
    list_init(list);
}

size_t list_size(list_t * list)
{
    return list->size;
}

int list_empty(list_t * list)
{
    return list_size(list) == 0;
}
