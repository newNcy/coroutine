#pragma once 

#include "container.h"

typedef enum 
{
    RB_COLOR_RED,
    RB_COLOR_BLACK
}rb_color_t;

typedef struct rb_node_t
{
    struct rb_node_t * parent;
    struct rb_node_t * left;
    struct rb_node_t * right;

    rb_color_t color;

    any_t key;
    any_t value;
}rb_node_t;

typedef struct 
{
    rb_node_t * root;
    any_compare_t less;
    any_compare_t equals;
}rb_tree_t;

typedef rb_tree_t map_t;
typedef rb_node_t * map_iterator_t;

void map_init(map_t * map, any_compare_t less, any_compare_t equals);
void map_destroy(map_t * map);

map_iterator_t map_set(map_t * map, any_t key, any_t value);
map_iterator_t map_find(map_t * map, any_t key);
void map_erase_key(map_t * map, any_t key);
void map_erase_iter(map_t * map, map_iterator_t iter);
int map_iterator_valid(map_t * map, map_iterator_t iter);
any_t map_iterator_get(map_iterator_t iter);
void map_iterator_set( map_iterator_t iter, any_t value);

map_iterator_t map_begin(map_t *map);
map_iterator_t map_end(map_t *map);
