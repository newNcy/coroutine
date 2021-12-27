#include "map.h"
#include <stdlib.h>

void rb_node_destroy(rb_node_t * node) 
{
    if (!node) {
        return;
    }
    rb_node_destroy(node->left);
    rb_node_destroy(node->right);

    node->left = nullptr;
    node->right = nullptr;

    free(node);
}

rb_node_t * rb_rotate_left(rb_node_t * node)
{
    if (!node) {
        return nullptr;
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
        return nullptr;
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
    rb_node_destroy(map->root);

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
    rb_node_t * node = ret;
    while (node->parent && node->parent->color == RB_COLOR_RED) {
        rb_node_t * parent = node->parent;
        rb_node_t * grandp = parent->parent;
        rb_node_t * uncle = grandp->left != parent? grandp->left : grandp->right;

        if (uncle && uncle->color == RB_COLOR_RED) {
            parent->color = RB_COLOR_BLACK;
            uncle->color = RB_COLOR_BLACK;
            grandp->color = RB_COLOR_RED;
            node = grandp;
        } else {
            if (node == parent->left && uncle == grandp->left) {
                grandp->right = rb_rotate_right(parent);
                node = parent;
            } else if (node == parent->right && uncle == grandp->right) {
                grandp->left = rb_rotate_left(parent);
                node = parent;
            } else {
                rb_node_t ** grandp_ptr = nullptr;
                if (!grandp->parent) {
                    grandp_ptr = &map->root;
                }else {
                    grandp_ptr == grandp->parent->left ? &(grandp->parent->left) : &(grandp->parent->right);
                }
                parent->color = RB_COLOR_BLACK;
                grandp->color = RB_COLOR_RED;
                if (node == parent->left && uncle == grandp->right) {
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

rb_node_t * rb_minimum(rb_node_t * root)
{
    while (root && root->left) {
        root = root->left;
    }
    return root;
}

void rb_swap_color(rb_node_t * a, rb_node_t * b)
{
	rb_color_t c = a->color;
	a->color = b->color;
	b->color = c;
}

rb_node_t ** rb_place_of(rb_tree_t * t, rb_node_t * node)
{
	if (node == t->root) {
		return &(t->root);
	}
	return node == node->parent->left? &(node->parent->left) : &(node->parent->right);
}

rb_node_t * rb_brother(rb_node_t * node) 
{
	if (!node || !node->parent) {
		return nullptr;
	}
	return node == node->parent->left ? node->parent->right : node->parent->left;
}

void rb_transplant(rb_tree_t * tree, rb_node_t * u, rb_node_t * v)
{
    if (tree->root == u) {
        tree->root = v;
    } else {
		if (u == u->parent->left) {
			u->parent->left = v;
		} else {
			u->parent->right = v;
		}
	}
    if (v) {
        v->parent = u->parent;
    }
}

void map_erase_iter(map_t * map, map_iterator_t iter)
{
    if (!iter) {
        return;
    }

    rb_node_t * y = iter;
    rb_color_t y_origin_color = iter->color;
    rb_node_t * x = nullptr;

    if (!iter->left) {
        x = iter->right;
        rb_transplant(map, iter, iter->right);
    } else if (!iter->right) {
        x = iter->left;
        rb_transplant(map, iter, iter->left);
    } else {
        y = rb_minimum(iter->right);
        y_origin_color = y->color;
        x = y->right;
        if (y->parent != iter) {
            rb_transplant(map, y, y->right);
            y->right = iter->right;
            y->right->parent = y;
        }

        rb_transplant(map, iter, y);
        y->left = iter->left;
        y->right->parent = y;
        y->color = iter->color;
    }

    if (y_origin_color == RB_COLOR_BLACK) {
        while (x != map->root && x->color == RB_COLOR_BLACK) {
			int left = x == x->parent->left;
			rb_node_t * p = x->parent;
			rb_node_t * w = rb_brother(x);
			rb_node_t ** parent_pos = rb_place_of(map, p);
			if (w->color == RB_COLOR_RED) {
				rb_swap_color(p, w);		
				if (left) {
					*parent_pos = rb_rotate_left(p);
				} else {
					*parent_pos = rb_rotate_right(p);
				}
				parent_pos = rb_place_of(map, p);
				w = rb_brother(x);
			}

			if (w->color == RB_COLOR_BLACK) {
				if (w->left->color == RB_COLOR_BLACK && w->right->color == RB_COLOR_BLACK) {
					w->color = RB_COLOR_RED;
					x = p;
				} else {
					if (left) {
						if (w->right->color == RB_COLOR_BLACK) {
							rb_swap_color(w, w->left);
							p->right = rb_rotate_right(w);
							w = p->right;
						} 

						w->color = p->color ;
						p->color = w->right->color = RB_COLOR_BLACK;
						*parent_pos = rb_rotate_left(p);
					} else {
						if (w->left->color == RB_COLOR_BLACK) {
							rb_swap_color(w, w->right);
							p->left= rb_rotate_left(w);
							w = p->left;
						}
						
						w->color = p->color ;
						p->color = w->right->color = RB_COLOR_BLACK;
						*parent_pos = rb_rotate_right(p);
					}
					x = map->root;
				}
			}
        }
		x->color = RB_COLOR_BLACK;
    }
	free(iter);
}

void map_erase_key(map_t * map, any_t key)
{
    if (!map || !map->less || !map->equals) {
        return;
    }

    map_iterator_t iter = map_find(map, key);
    map_erase_iter(map, iter);
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

int map_iterator_valid(map_t * map, map_iterator_t iter)
{
    return iter != nullptr;
}

any_t map_iterator_get( map_iterator_t iter)
{
    return iter->value;
}

void map_iterator_set( map_iterator_t iter, any_t value)
{
    iter->value = value;
}
