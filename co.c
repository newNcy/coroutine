#include <stdio.h>
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
    co_timer_t * tail;
}co_timer_mgr_t;

co_timer_mgr_t co_timer_mgr = {0};


void sleep(int s)
{
    if (s <= 0) {
        return;
    }
    co_timer_t * timer = (co_timer_t*)malloc(sizeof(co_timer_t));
    timer->co_id = co_running();
    timer->expiration_time = time(0) + s;
    timer->next = NULL;

    if (!co_timer_mgr.head) {
        co_timer_mgr.tail = co_timer_mgr.head = timer;
    } else {
        co_timer_mgr.tail->next = timer;
        co_timer_mgr.tail = timer;
    }
    co_yield();
}

void foo()
{
    for (int i = 0; i < 3; ++ i) {
        sleep(1);
        printf("sleep 1s\n");
        fflush(stdout);
    }
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
            co_resume(cur->co_id);
            co_timer_t * next = cur->next;
            free(cur);
            cur = next;
        } else {
            break;
        }
    }
    co_timer_mgr.head = cur;
    if (!cur) {
        co_timer_mgr.tail = NULL;
    }
}


int main()
{

    co_init(10);

    int co = co_create((coroutine_entry_t)foo);
    co_resume(co);

    while (has_timer()) {
        process_timer();
    }
    co_finish();
    return 0;
}
