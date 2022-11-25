#include "timer.h"
#include "heap.h"
#include "map.h"
#include "list.h"
#include "aio.h"
#include "coroutine.h"


int timeval_less(struct timeval * lhs, struct timeval * rhs)
{
    return lhs->tv_sec < rhs->tv_sec || lhs->tv_sec == rhs->tv_sec && lhs->tv_usec < rhs->tv_usec;
}

int timer_compare(co_timer_t * lhs, co_timer_t * rhs)
{
    return timeval_less(&lhs->expiration_time, &rhs->expiration_time);
}

int usleep(useconds_t us)
{
    if (us < 0) {
        return 0;
    }
    co_timer_t * timer = (co_timer_t*)malloc(sizeof(co_timer_t));
    timer->co_id = co_running();
    gettimeofday(&timer->expiration_time, NULL);
    timer->expiration_time.tv_sec += us/1000000;
    timer->expiration_time.tv_usec += us%1000000;
    heap_push(&thread_env()->timer_mgr, timer);
    co_yield();
    return 1;
}

unsigned int sleep(unsigned int s)
{
    return usleep(s * 1000 * 1000);
}

suseconds_t process_timer()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    while (!heap_empty(&thread_env()->timer_mgr)) {
        co_timer_t * timer = (co_timer_t*)heap_top(&thread_env()->timer_mgr);
        if (timeval_less(&now, &timer->expiration_time)) {
            return (timer->expiration_time.tv_sec - now.tv_sec) * 1000 + (timer->expiration_time.tv_usec - now.tv_usec)/1000;
        } else {
            int id = timer->co_id;
            free(timer);
            heap_pop(&thread_env()->timer_mgr);

            co_resume(id);
        }
    }
    return -1;
}


void co_event_init()
{
    heap_init(&thread_env()->timer_mgr, timer_compare);
    io_init();
}

void co_event_loop()
{
    while (1) {
        suseconds_t next_wake = process_timer();
        io_update(next_wake);
        if (co_is_all_finish()) {
            break;
        }
    }
    heap_destroy(&thread_env()->timer_mgr);
}




