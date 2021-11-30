#include "heap.h"

void heap_init(heap_t * heap, any_compare_t compare)
{
    array_init(&heap->array);
    heap->compare = compare;
}

int heap_push(heap_t * heap, any_t any)
{
    if (!array_push_back(&heap->array, any)) {
        return 0;
    }
    int idx = heap->array.size - 1;

    while (idx > 0) {
        int parent = (idx-1)/2;
        any_t parent_val = array_get(&heap->array, parent);
        if (!heap->compare(parent, any)) {
            array_set(&heap->array, idx, parent_val);
        } else {
            break;
        }
        idx = parent;
    }

    array_set(&heap->array, idx, any);
    return 1;
}

any_t heap_top(heap_t * heap)
{
    return array_get(&heap->array, 0);
}

int heap_pop(heap_t * heap)
{
}

int heap_size(heap_t * heap)
{
    return heap->array.size;
}

int heap_empty(heap_t * heap)
{
    return heap->array.size != 0;
}
