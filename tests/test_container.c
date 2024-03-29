#include "map.h"
#include "heap.h"
#include "list.h"
#include <string.h>

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

    for (map_iterator_t iter = map_begin(map); iter !=  map_end(map); iter = map_next(iter)) {
        //printf("%d ", map_iterator_get(iter));
    }
    printf("\n");
}

int compre_str(any_t a, any_t b)
{
    return strcmp((char*)a, (char*)b) < 0;
}


int main()
{
    /*
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

    */
    map_t * map = map_create(NULL, NULL);

    for (int i = 3; i < 14; i ++) {
        map_set(map, (any_t)i, (any_t)i);
    }
    map_remove_key(map, 13);
    map_remove_key(map, 6);
    map_remove_key(map, 7);
    map_remove_key(map, 8);
    map_remove_key(map, 9);

    /*
    for (int i = 7; i >= 1; -- i) {
        map_remove_key(&map, i);
        tree_print(map.root);
        printf("------------------------------\n");
    }
    */

}


