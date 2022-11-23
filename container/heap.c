#include "heap.h"

void heap_init(heap_t * heap, any_compare_t compare)
{
    array_init(&heap->array);
    heap->compare = compare;
}

void heap_destroy(heap_t * heap)
{
    array_destroy(&heap->array);
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
        if (!heap->compare(parent_val, any)) {
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

void heap_pop(heap_t * heap)
{
    if (heap_empty(heap)) {
        return;
    }

    array_set(&heap->array, 0, array_get(&heap->array, heap->array.size-1));
    array_erase(&heap->array, heap->array.size - 1);

    int idx = 0;
    int last = heap->array.size - 1;
    any_t val = array_get(&heap->array, idx);
    while (idx < last) {
        int child = idx * 2 + 1;
        if (child > last) {
            break;
        }
        any_t child_val = array_get(&heap->array, child);
        if ( child + 1 <= last) {
            any_t rigth_val = array_get(&heap->array, child);
            if (heap->compare(rigth_val, child_val)) {
                child ++;
                child_val = rigth_val;
            }
        }

        if (heap->compare(child_val, val)) {
            array_set(&heap->array, idx, child_val);
            idx = child;
        }else {
            break;
        }
    }

    array_set(&heap->array, idx, val);
}

int heap_size(heap_t * heap)
{
    return heap->array.size;
}

int heap_empty(heap_t * heap)
{
    return heap_size(heap) == 0;
}
