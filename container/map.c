#include "map.h"
#include <stdlib.h>
#include "list.h"


int item_w = 1;

int space_forh(int h)
{
    if (h == 1) return 0;
    return 2 * space_forh(h-1) + item_w;
}

void space(int c)
{
    for (int i = 0; i < c; ++ i) {
        printf(" ");
    }
}

int tree_height(rb_node_t * node)
{
    if (!node) return 0;
    int lh = 1 + tree_height(node->left);
    int rh = 1 + tree_height(node->right);
    return lh > rh ? lh : rh;
}

void tree_print(rb_node_t * node)
{
    if (!node) {
        return;
    }
    while (node->parent) node = node->parent;

    list_t * cur = list_create();
    list_t * nxt = list_create();

    list_push_back(cur, node);
    int height = tree_height(node);
    int valid = 1;
    while (valid) {
        valid = 0;
        int sp = space_forh(height);
        int sp2 = space_forh(height+1);
        space(sp);
        while (!list_empty(cur)) {
            rb_node_t * n = (rb_node_t*)list_front(cur);
            list_pop_front(cur);
            if (n) {
                if (rb_node_is_red(n)) {
                    printf("\e[31m%d\e[0m", n->key);
                } else {
                    printf("%d", n->key);
                }
                list_push_back(nxt, n->left);
                list_push_back(nxt, n->right);
                if (n->left) valid ++;
                if (n->right) valid ++;
            } else {
                printf(" ");
                list_push_back(nxt, NULL);
                list_push_back(nxt, NULL);
            }
            space(sp2);
        }
        list_t * tmp = cur;
        cur = nxt;
        nxt = tmp;
        height --;
        printf("\n");
    }
}


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

void rb_node_set_red(rb_node_t * node, int is_red)
{
    node->color = is_red ? RB_COLOR_RED : RB_COLOR_BLACK;
}

void rb_swap_color(rb_node_t * a, rb_node_t * b)
{
	rb_color_t c = a->color;
	a->color = b->color;
	b->color = c;
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
    rb_swap_color(right, node);
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
    rb_swap_color(left, node);
    return left;
}


int rb_node_is_red(rb_node_t* node)
{
    return node && node->color == RB_COLOR_RED;
}

void rb_node_flip_color(rb_node_t * n)
{
    if (n) rb_node_set_red(n, !rb_node_is_red(n));
    if (n->left) rb_node_set_red(n->left, !rb_node_is_red(n->left));
    if (n->right) rb_node_set_red(n->right, !rb_node_is_red(n->right));
}

void rb_node_init(rb_node_t * node)
{
    node->parent = nullptr;
    node->left = nullptr;
    node->right = nullptr;
    node->key = nullptr;
    node->value = nullptr;
    rb_node_set_red(node, 1);
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
    map->less = less;
    map->equals = equals;
    map->size = 0;
}


void map_destroy(map_t * map)
{
    rb_node_destroy(map->root);

    map_init(map, nullptr, nullptr);
}

rb_node_t ** rb_place_of(rb_tree_t * t, rb_node_t * node)
{
	if (node == t->root) {
		return &(t->root);
	}
	return node == node->parent->left? &(node->parent->left) : &(node->parent->right);
}

rb_node_t * rb_fixup(rb_node_t * n)
{
    if (rb_node_is_red(n->right) && !rb_node_is_red(n->left)) {
        n = rb_rotate_left(n);
    }
    if (rb_node_is_red(n->left) && rb_node_is_red(n->left->left)) {
        n = rb_rotate_right(n);
    }
    if (rb_node_is_red(n->left) && rb_node_is_red(n->right)) {
        rb_node_flip_color(n);
    }
    return n;
}

rb_node_t * rb_put(rb_node_t * n, any_t key, any_t value, any_compare_t less, rb_node_t ** ret)
{
    if (!n) return *ret = rb_node_create(key, value);
    if (less(key, n->key)) n->left = rb_put(n->left, key, value, less, ret);
    else if (less(n->key, key)) n->right = rb_put(n->right, key, value, less, ret);
    else n->value = value;

    return rb_fixup(n); 
}


map_iterator_t map_set(map_t * map, any_t key, any_t value)
{
    if (!map || !map->less || !map->equals) {
        return nullptr;
    }

    map_iterator_t ret;
    map->root = rb_put(map->root, key, value, map->less, &ret);
    rb_node_set_red(map->root, 0);
    map->size++;
    return ret;
}

rb_node_t * rb_minimum(rb_node_t * root)
{
    while (root && root->left) {
        root = root->left;
    }
    return root;
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

    if (iter == map->root && map->size == 1) {
        map->root = nullptr;
        map->size --;
        free(iter);
        return;
    }

    rb_node_t * y = iter;
    rb_color_t y_origin_color = iter->color;
    rb_node_t * x = nullptr;

    rb_node_t null_holder;
    null_holder.left = nullptr;
    null_holder.right = nullptr;
    rb_node_set_red(&null_holder, 0);

    if (!iter->left) {
        x = iter->right;
        rb_transplant(map, iter, iter->right);
    } else if (!iter->right) {
        x = iter->left;
        rb_transplant(map, iter, iter->left);
    } else {
        y = rb_minimum(iter->right);
        y_origin_color = y->color;
        if (!y->right) {
            y->right = &null_holder; 
            null_holder.parent = y;
        }
        x = y->right;
        if (y->parent != iter) {
            rb_transplant(map, y, y->right);
            y->right = iter->right;
            y->right->parent = y;
        }

        rb_transplant(map, iter, y);
        y->left = iter->left;
        y->left->parent = y;
        y->color = iter->color;
    }

	/* rb-delete-fixup */
    if (!y_origin_color == RB_COLOR_BLACK) {
        while (x != map->root &&  x->color == RB_COLOR_BLACK) {
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
                    if (x == &null_holder) {
                        *rb_place_of(map, x) = nullptr;
                    }
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
                    if (x == &null_holder) {
                        *rb_place_of(map, x) = nullptr;
                    }
					x = map->root;
				}
			}
        }
		x->color = RB_COLOR_BLACK;
    }
    map->size --;
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

map_iterator_t map_begin(map_t *map)
{
    return rb_minimum(map->root);
}

map_iterator_t map_end(map_t *map)
{
    return nullptr;
}

map_iterator_t iter_next(map_iterator_t iter)
{
    if (!iter) {
        return nullptr;
    }
    if (iter->right) {
        map_iterator_t next = rb_minimum(iter->right);
        if (next) {
            return next;
        }
    }

    if (iter->parent && iter->parent->left == iter) {
        return iter->parent;
    }

    while (iter->parent && iter == iter->parent->right) {
        iter = iter->parent;
    }

    return iter->parent;
}

size_t map_size(map_t * map)
{
    return map->size;
}

rb_node_t * rb_move_red_left(rb_node_t * node) 
{
    rb_node_flip_color(node);
    if (node->right && rb_node_is_red(node->right->left)) {
        node->right = rb_rotate_right(node->right);
        node = rb_rotate_left(node);
        rb_node_flip_color(node);
    }

    return node;
}

rb_node_t * rb_move_red_right(rb_node_t * node)
{
    rb_node_flip_color(node);
    // 同样这里两边：
    if (node->left && rb_node_is_red(node->left->left)) {
        node = rb_rotate_right(node);
        rb_node_flip_color(node);
    }

    return node;
}

rb_node_t * rb_remove_min(rb_node_t * node, rb_node_t ** out)
{
    if (!node->left) {
        *out = node;
        return nullptr;
    }

    if (!rb_node_is_red(node->left) && !rb_node_is_red(node->left->left)) {
        rb_move_red_left(node);
    }

    node->left = rb_remove_min(node->left, out);
    node = rb_fixup(node);
    return node;
}

rb_node_t * rb_remove(rb_node_t * node, any_t key, any_compare_t less, rb_node_t ** out)
{
    if (!node) {
        return nullptr;
    }

    if (less(key, node->key)) {
        if (node->left) {
            if (!rb_node_is_red(node->left) && node->left && !rb_node_is_red(node->left->left)) {     
                node = rb_move_red_left(node);
            }
            node->left = rb_remove(node->left, key, less, out);
        }
    } else {
        if (rb_node_is_red(node->left)) {
            node = rb_rotate_right(node);
        }
        int eq = !less(node->key, key);
        if (eq && node->right == nullptr) {
            *out = node;
            return nullptr;
        }

        if (!rb_node_is_red(node->right) && node->right && !rb_node_is_red(node->right->left)) {
            node = rb_move_red_right(node);
        }

        eq = !less(node->key, key);
        if (eq) {
            rb_node_t * min_node = rb_minimum(node->right);
            node->key = min_node->key;
            node->value = min_node->value;
            node->right = rb_remove_min(node->right, out);
        }else {
            node->right = rb_remove(node->right, key, less, out);
        }
    }

    return node ? rb_fixup(node) : node;
}

void map_remove_min(map_t * map)
{
    if (map->root) {
        rb_node_t* to_remove = nullptr;
        map->root = rb_remove_min(map->root,&to_remove);
        if (map->root) {
            rb_node_set_red(map->root, 0);
        }
        if (to_remove) {
            free(to_remove);
            map->size--;
        }
    }
}

void map_remove_key(map_t * map, any_t key)
{
    rb_node_t* to_remove = nullptr;
    map->root = rb_remove(map->root, key, map->less, &to_remove);
    if (map->root) {
        rb_node_set_red(map->root, 0);
    }
    if (to_remove) {
        free(to_remove);
        map->size--;
    }
}

