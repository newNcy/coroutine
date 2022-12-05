#include "array.h"
#include <stdlib.h>
#include <string.h>

void array_init(array_t * array)
{
    array->size = 0;
    array->capacity = 0;
    array->data = NULL;
}

array_t * array_create()
{
    array_t * array = (array_t*)malloc(sizeof(array_t));
    array_init(array);
    return array;
}

any_t * array_begin(array_t * array)
{
    return array->data;
}

any_t * array_end(array_t * array)
{
    return array->data + array->size;
}

any_t array_get(array_t * array, int index)
{
    return array->data[index];
}

void array_set(array_t * array, int index, any_t any)
{
    array->data[index] = any;
}

void array_reserve(array_t * array, size_t size)
{
    any_t * data = (any_t*)malloc(size * sizeof(any_t));
    if (data) {
        if (array->size) {
            memcpy(data, array->data, array->size * sizeof(any_t));
            free(array->data);
        }
        array->data = data;
        array->capacity = size;
    }
}
void array_resize(array_t * array, size_t size)
{
    if (size >  array->capacity) {
        array_reserve(array, size);
    }
    if (array->capacity >= size) {
        array->size = size;
    }
}

int array_push(array_t * array, any_t any)
{
    if (array->size == array->capacity) {
        array_reserve(array, array->capacity ? array->capacity * 2:10);
    }

    if (array->size < array->capacity) {
        array->data[array->size ++] = any;
        return 1;
    }

    return 0;
}

int array_insert(array_t * array, int index, any_t any)
{
    if (index < 0 || index > array->size) {
        return 0;
    }
    if (!array_push(array, any)) {
        return 0;
    }

    if (index != array->size - 1) {
        any_t any = array_get(array, array->size-1);
        int i = 0;
        for (i = array->size - 1; i > index; ++i) {
            array_set(array, i, array_get(array, i-1));
        }
        array_set(array, i, any);
    }
    return 1;
}

int array_erase(array_t * array, int index) 
{
    if (index < 0 || index >= array->size) {
        return 0;
    }
    for (; index < array->size - 1; ++ index) {
        array_set(array, index, array_get(array, index + 1));
    }
    array->size --;
    return 1;
}

void array_destroy(array_t * array)
{
    if (array->data) {
        free(array->data);
    }
    array_init(array);
    free(array);
}
