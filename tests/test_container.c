#include "map.h"

int compare(any_t a, any_t b)
{
    return a < b;
}

int compare_e(any_t a, any_t b)
{
    return a == b;
}

int main()
{
    map_t map;
    map_init(&map, compare, compare_e);
    map_set(&map, 1, 1);
    map_set(&map, 2, 2);
    map_set(&map, 3, 3);
    map_set(&map, 4, 4);
    map_set(&map, 5, 6);
    map_set(&map, 7, 7);
    map_set(&map, 8, 8);

	map_erase_key(&map, 2);
    map_iterator_t iter = map_find(&map, 2);
    if (iter) {
        printf("%d\n", map_iterator_get(iter));
    }

    map_destroy(&map);
}


