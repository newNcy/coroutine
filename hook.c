#include "coroutine.h"

#include <time.h>
#include <malloc.h>

typedef struct co_timer_t 
{
    int co_id;
    time_t expiration_time;
    struct co_timer_t * next;
}co_timer_t;

typedef struct co_timer_mgr_t
{
    co_timer_t * head;
}co_timer_mgr_t;

co_timer_mgr_t co_timer_mgr = {0};


void print_tmgr()
{
    co_timer_t * cur = co_timer_mgr.head;
    int max = 20;
    int i = 0; 
    while (cur) {
        ++ i;
        if (i >= max) {
            printf("xxx\n");
            break;
        }
        printf("[%d:%d]\n", cur->co_id, cur->expiration_time);
        cur = cur->next;
    }
}

void sleep(int s)
{
    if (s <= 0) {
        return;
    }
    co_timer_t * timer = (co_timer_t*)malloc(sizeof(co_timer_t));
    timer->co_id = co_running();
    timer->expiration_time = time(0) + s;
    timer->next = NULL;

    //printf("%d sleep %d\n", timer->co_id, s);
    //print_tmgr();
    if (!co_timer_mgr.head) {
        //printf("first one\n");
        co_timer_mgr.head = timer;
    } else {
        // todo 换成小根堆
        if (co_timer_mgr.head->expiration_time > timer->expiration_time) {
            //printf("insert as head\n");
            timer->next = co_timer_mgr.head;
            co_timer_mgr.head = timer;
        }else {
            //printf("insert inside\n");
            co_timer_t * cur = co_timer_mgr.head;
            while (cur->next && cur->next->expiration_time < timer->expiration_time) {
                cur = cur->next;
            }
            timer->next = cur->next;
            cur->next = timer;
        }
    }
    //print_tmgr();
    co_yield();
}

int has_timer()
{
    return co_timer_mgr.head != NULL;
}

void process_timer()
{
    co_timer_t * cur = co_timer_mgr.head;
    time_t now = time(0);
    while (cur) {
        if (cur->expiration_time <= now) {
            int id = cur->co_id;
            co_timer_t * next = cur->next;
            free(cur);
            co_timer_mgr.head = cur = next;

            co_resume(id);
        } else {
            break;
        }
    }
}


void co_event_loop()
{
    while (has_timer()) {
        process_timer();
    }
}

