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

int tree_height(rb_node_t * node)
{
    if (!node) return 0;
    int lh = 1 + tree_height(node->left);
    int rh = 1 + tree_height(node->right);
    return lh > rh ? lh : rh;
}

int item_w = 3;

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


void tree_print(rb_node_t * node)
{
    if (!node) {
        return;
    }
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
                    printf("\e[31m%03d\e[0m", n->key);
                } else {
                    printf("%03d", n->key);
                }
                list_push_back(nxt, n->left);
                list_push_back(nxt, n->right);
                if (n->left) valid ++;
                if (n->right) valid ++;
            } else {
                printf("   ");
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
    for (int i = 1; i <= 11; ++ i) {
        map_set(&map, i, i);
        tree_print(map.root);
        printf("\n");
    }

    for (int i = 1; i <= 11; ++ i) {
        map_erase_key(&map, i);
        tree_print(map.root);
        printf("------------------------------\n");
    }
    
}


