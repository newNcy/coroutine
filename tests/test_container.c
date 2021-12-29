#include "map.h"

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
    map_t map;
    map_init(&map, compare, compare_e);
    insert_and_print(&map, 1, 1);
    insert_and_print(&map, 2, 2);
    insert_and_print(&map, 3, 3);
    insert_and_print(&map, 4, 4);
    insert_and_print(&map, 5, 5);
    insert_and_print(&map, 6, 6);
    insert_and_print(&map, 7, 7);
    insert_and_print(&map, 8, 8);

	map_erase_key(&map, 2);
    map_iterator_t iter = map_find(&map, 2);

    map_destroy(&map);
}


