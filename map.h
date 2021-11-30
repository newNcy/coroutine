#pragma once 

#include "container.h"

enum rb_color_t
{
    RB_COLOR_RED,
    RB_COLOR_BLACK
};

typedef struct rb_node_t
{
    struct rb_node_t * parent;
    struct rb_node_t * left;
    struct rb_node_t * right;

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

void map_init(map_t * map);
void map_destroy(map_t * map);

void map_set(map_t * map, any_t key, any_t value);
map_iterator_t map_find(map_t * map, any_t key);
int map_iterator_valid(map_t * map, map_iterator_t iter);
any_t map_get(map_t * map, map_iterator_t iter);
