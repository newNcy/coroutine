#include "heap.h"
#include <stdlib.h>

void heap_init(heap_t * heap, any_compare_t compare)
{
    heap->array = array_create();
    heap->compare = compare;
}

heap_t * heap_create(any_compare_t compare)
{
    heap_t * heap = (heap_t*)malloc(sizeof(heap_t));
    heap_init(heap, compare);
    return heap;
}

void heap_destroy(heap_t * heap)
{
    array_destroy(heap->array);
    free(heap);
}

void print_heap(heap_t * heap)
{
    for (int i = 0; i  < array_size(heap->array); ++ i) {
        printf("%p ", array_get(heap->array, i));
    }
    printf("\n");
}

int heap_push(heap_t * heap, any_t any)
{
    if (!array_push(heap->array, any)) {
        return 0;
    }
    int idx = array_size(heap->array)- 1;

    while (idx > 0) {
        int parent = (idx-1)/2;
        any_t parent_val = array_get(heap->array, parent);
        if (!heap->compare(parent_val, any)) {
            array_set(heap->array, idx, parent_val);
        } else {
            break;
        }
        idx = parent;
    }

    array_set(heap->array, idx, any);
    return 1;
}

any_t heap_top(heap_t * heap)
{
    return array_get(heap->array, 0);
}

void heap_pop(heap_t * heap)
{
    if (heap_empty(heap)) {
        return;
    }

    array_set(heap->array, 0, array_get(heap->array, array_size(heap->array)-1));
    array_erase(heap->array, array_size(heap->array) - 1);

    int idx = 0;
    int last = array_size(heap->array)- 1;
    any_t val = array_get(heap->array, idx);
    while (idx < last) {
        int child = idx * 2 + 1;
        if (child > last) {
            break;
        }
        any_t child_val = array_get(heap->array, child);
        if ( child + 1 <= last) {
            any_t rigth_val = array_get(heap->array, child + 1);
            if (heap->compare(rigth_val, child_val)) {
                child ++;
                child_val = rigth_val;
            }
        }

        if (heap->compare(child_val, val)) {
            array_set(heap->array, idx, child_val);
            idx = child;
        }else {
            break;
        }
    }

    array_set(heap->array, idx, val);
}

int heap_size(heap_t * heap)
{
    return array_size(heap->array);
}

int heap_empty(heap_t * heap)
{
    return heap_size(heap) == 0;
}
