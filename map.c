#include "map.h"

void map_init(map_t * map)
{
    map->root = nullptr;
    map->less = nullptr;
    map->equals = nullptr;
}


void rb_node_free(rb_node_t * node) 
{
    if (!node) {
        return;
    }
    rb_node_free(node->left);
    rb_node_free(node->right);

    node->left = nullptr;
    node->right = nullptr;

    free(node);
}

void map_destroy(map_t * map)
{
    rb_node_free(map->root);

    map_init(map);
}

void map_set(map_t * map, any_t key, any_t value)
{
    map_iterator_t iter = map_find(map, key);
    if (map_iterator_valid(map, iter)) {
        iter->value = value;
    } else {
    }
}
