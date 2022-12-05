#pragma once 
#include "array.h"

typedef struct 
{
    any_compare_t compare;
    array_t * array;
} heap_t;

void heap_init(heap_t * heap, any_compare_t compare);
void heap_destroy(heap_t * heap);
int heap_push(heap_t * heap, any_t any);
any_t heap_top(heap_t * heap);
void heap_pop(heap_t * heap);
int heap_size(heap_t * heap);
int heap_empty(heap_t * heap);
heap_t * heap_create(any_compare_t compare);
