#include "map.h"
#include "heap.h"

int compare(any_t a, any_t b)
{
    return a < b;
}

int compare_e(any_t a, any_t b)
{
    return a == b;
}

void insert_and_print(map_t * map, any_t key, any_t value)
{
    map_set(map, key, value);

    for (map_iterator_t iter = map_begin(map); iter !=  map_end(map); iter = iter_next(iter)) {
        printf("%d ", map_iterator_get(iter));
    }
    printf("\n");
}

int main()
{
    heap_t heap;
    heap_init(&heap, compare);
    for (int i = 1; i <= 10; ++ i) {
        heap_push(&heap, i);
    }
    for (int i = 1; i <= 10; ++ i) {
        int t = heap_top(&heap);
        printf("%d\n", t);
        heap_pop(&heap);
    }
    heap_destroy(&heap);
}


