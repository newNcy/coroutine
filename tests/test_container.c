#include "map.h"
#include "heap.h"
#include "list.h"

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
    map_t map;
    map_init(&map, compare, compare_e);
    map_set(&map, 5, 4);
    printf("++++++++++++++++++++++++++++\n");
    map_set(&map, -1, 4);
    printf("++++++++++++++++++++++++++++\n");
    map_set(&map, 6, 4);
    printf("++++++++++++++++++++++++++++\n");
    map_set(&map, 4, 4);
    printf("++++++++++++++++++++++++++++\n");

    map_remove_key(&map, 6);
    tree_print(map.root);

    /*
    for (int i = 7; i >= 1; -- i) {
        map_remove_key(&map, i);
        tree_print(map.root);
        printf("------------------------------\n");
    }
    */

    list_t * list = list_create();
    printf("%x %x %d", list_begin(list), list_end(list), list_empty(list));
}


