#include "map.h"

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

rb_node_t * rb_rotate_left(rb_node_t * node)
{
    if (!node) {
        return;
    }
    rb_node_t * left = node->left;
    rb_node_t * right = node->right;
    if (!right) {
        return node;
    }

    node->right = right->left;
    right->left = node;

    right->parent = node->parent;
    node->parent = right;
    return right;
}

rb_node_t * rb_rotate_right(rb_node_t * node)
{
    if (!node) {
        return;
    }
    rb_node_t * left = node->left;
    rb_node_t * right = node->right;
    if (!left) {
        return node;
    }

    node->left = left->right;
    left->right = node;
    
    left->parent = node->parent;
    node->parent = left;
    return left;
}

void rb_node_init(rb_node_t * node)
{
    node->parent = nullptr;
    node->left = nullptr;
    node->right = nullptr;
    node->key = nullptr;
    node->value = nullptr;
    node->color = RB_COLOR_RED;
}

rb_node_t * rb_node_create(any_t key, any_t value)
{
    rb_node_t * node = (rb_node_t*)malloc(sizeof(rb_node_t));
    rb_node_init(node);
    node->key = key;
    node->value = value;
    return node;
}

void rb_node_destroy(rb_node_t * node)
{
    rb_node_init(node);
    free(node);
}

void map_init(map_t * map, any_compare_t less, any_compare_t equals)
{
    map->root = nullptr;
    map->less = nullptr;
    map->equals = nullptr;
    map->less = less;
    map->equals = equals;
}


void map_destroy(map_t * map)
{
    rb_node_free(map->root);

    map_init(map, nullptr, nullptr);
}

map_iterator_t map_set(map_t * map, any_t key, any_t value)
{
    if (!map || !map->less || !map->equals) {
        return nullptr;
    }

    if (!map->root) {
        rb_node_t * node = rb_node_create(key, value);
        node->color = RB_COLOR_BLACK;
        map->root = node;
        return map->root;
    }

    rb_node_t * iter = map->root;
    any_compare_t less = map->less;
    any_compare_t equals = map->equals;

    map_iterator_t ret = nullptr;

    while (iter) {
        if (equals(key, iter->key)) {
            return iter;
        }
        rb_node_t ** next = nullptr;
        if (less(key, iter->key)) {
            next = &iter->left;
        } else {
            next = &iter->right;
        }
            
        if (!*next) {
            rb_node_t * node = rb_node_create(key, value);
            *next = node;
            node->parent = iter;
            ret = node;
            break;
        }
        
        iter = *next;
    }

    /* insert-fixup */
    
    while (ret->parent->color == RB_COLOR_RED) {
        rb_node_t * parent = ret->parent;
        rb_node_t * grandp = parent->parent;
        rb_node_t * uncle = grandp->left != parent? grandp->left : grandp->right;

        if (uncle && uncle->color == RB_COLOR_RED) {
            parent->color = RB_COLOR_BLACK;
            uncle->color = RB_COLOR_BLACK;
            grandp->color = RB_COLOR_RED;
            parent = grandp->parent;
        } else {
            if (ret == parent->left && uncle == grandp->left) {
                grandp->right = rb_rotate_right(parent);
                ret = parent;
            } else if (ret == parent->right && uncle == grandp->right) {
                grandp->left = rb_rotate_left(parent);
                ret = parent;
            } else {
                rb_node_t ** grandp_ptr = nullptr;
                if (!grandp->parent) {
                    grandp_ptr = &map->root;
                }else {
                    grandp == grandp->parent->left ? &grandp->parent->left : &grandp->parent->right;
                }
                parent->color = RB_COLOR_BLACK;
                grandp->color = RB_COLOR_RED;
                if (ret == parent->left && uncle == grandp->right) {
                    *grandp_ptr = rb_rotate_right(grandp);
                } else {
                    *grandp_ptr = rb_rotate_left(grandp);
                }
            }
        }
    }
    if (map->root->color == RB_COLOR_RED) {
        map->root->color = RB_COLOR_BLACK;
    }
    return ret;
}

map_iterator_t map_find(map_t * map, any_t key)
{
    if (!map || !map->less || !map->equals) {
        return nullptr;
    }

    any_compare_t less = map->less;
    any_compare_t equals = map->equals;

    rb_node_t * iter = map->root;
    while (iter) {
        if (equals(key, iter->key)) {
            break;
        }
        if (less(key, iter->key)) {
            iter = iter->left;
        } else {
            iter = iter->right;
        }
    }
    return iter;
}

any_t map_iterator_get( map_iterator_t iter)
{
    return iter->value;
}

void map_iterator_set( map_iterator_t iter, any_t value)
{
    iter->value = value;
}
