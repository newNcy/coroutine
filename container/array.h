#pragma once
#include "container.h"

typedef void * any_t;
typedef struct 
{
	int size;
	int capacity;
	any_t * data;
}array_t;

array_t * array_create();
void array_destroy(array_t * array);
any_t * array_begin(array_t * array);
any_t * array_end(array_t * array);
any_t array_get(array_t * array, int index);
void array_set(array_t * array, int index, any_t any);

int array_push(array_t * array, any_t any);
int array_insert(array_t * array, int index, any_t any);
int array_erase(array_t * array, int index);
void array_resize(array_t * array, size_t size);
void array_reserve(array_t * array, size_t size);

static inline int array_size(array_t * array) { return array->size; }
