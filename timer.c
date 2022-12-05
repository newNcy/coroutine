#include "timer.h"
#include "heap.h"
#include "map.h"
#include "list.h"
#include "aio.h"
#include "coroutine.h"
#include <stdlib.h>


int timeval_less(struct timeval * lhs, struct timeval * rhs)
{
    return lhs->tv_sec < rhs->tv_sec || lhs->tv_sec == rhs->tv_sec && lhs->tv_usec < rhs->tv_usec;
}

int timer_compare(co_timer_t * lhs, co_timer_t * rhs)
{
    return timeval_less(&lhs->expiration_time, &rhs->expiration_time);
}

int usleep(long long  us)
{
    if (us < 0) {
        return 0;
    }
    co_timer_t * timer = (co_timer_t*)malloc(sizeof(co_timer_t));
    timer->co_id = co_running();
    gettimeofday(&timer->expiration_time, NULL);
    timer->expiration_time.tv_sec += us/1000000;
    timer->expiration_time.tv_usec += us%1000000;
    heap_push(thread_env()->timer_mgr, timer);
    co_yield();
    return 1;
}

unsigned int sleep(unsigned int s)
{
    return usleep(s * 1000 * 1000);
}

#ifdef WIN32
int gettimeofday(struct timeval* tp, struct timezone* tzp)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
    // until 00:00:00 January 1, 1970 
    static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;

    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    time = ((uint64_t)file_time.dwLowDateTime);
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tp->tv_sec = (long)((time - EPOCH) / 10000000L);
    tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
    return 0;
}
#endif

long long process_timer()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    while (!heap_empty(thread_env()->timer_mgr)) {
        co_timer_t * timer = (co_timer_t*)heap_top(thread_env()->timer_mgr);
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





