#include "timer.h"
#include "heap.h"
#include "map.h"
#include "list.h"
#include "aio.h"
#include "coroutine.h"
#include <stdlib.h>


int timer_less(co_timer_t * lhs, co_timer_t * rhs)
{
    return lhs->ns < rhs->ns;
}

heap_t * timer_mgr_init()
{
    return heap_create((any_compare_t)timer_less);
}

int co_sleep_ns(long long ns_delay)
{
    if (ns < 0) {
        return 0;
    }
    co_timer_t * timer = (co_timer_t*)malloc(sizeof(co_timer_t));
    timer->co = co_running();
    timer->ns = ns() + ns_delay;
    heap_push(thread_env()->timer_mgr, timer);
    co_yield(NULL);
    return 1;
}



long long process_timer()
{
    uint64_t now = ns();
    while (!heap_empty(thread_env()->timer_mgr)) {
        co_timer_t * timer = (co_timer_t*)heap_top(thread_env()->timer_mgr);
        if (now < timer->ns) {
            return (timer->ns - now)/1000000;
        } else {
            co_t * co = timer->co;
            free(timer);
            heap_pop(thread_env()->timer_mgr);
            co_resume(co);
        }
    }
    return -1;
}





