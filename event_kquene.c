
#include "array.h"
#include <sys/event.h>
#include "aio.h"
#include "container/array.h"
#include <unistd.h>
#include <stdlib.h>


typedef struct kevent kevent_t;
struct event_ctx_t
{
    int fd;
    kevent_t * events;
    int event_count;
    int event_cap;
};

event_ctx_t * event_create()
{
    event_ctx_t * ctx = (event_ctx_t*)malloc(sizeof(event_ctx_t));
    ctx->fd = kqueue();
    ctx->event_count = 0;
    ctx->event_cap = 10;
    ctx->events = (kevent_t*)malloc(sizeof(kevent_t)*ctx->event_cap);
    return ctx;
}

void event_destroy(event_ctx_t * ctx)
{
    free(ctx->events);
    free(ctx);
}

void event_add(event_ctx_t * ctx, int fd)
{
    struct kevent change;
    EV_SET(&change, fd, EVFILT_READ | EVFILT_WRITE | EVFILT_EXCEPT, EV_ADD, 0, 0, NULL);
    kevent(ctx->fd, &change, 1,NULL, 0, NULL);
    if (ctx->event_count  == ctx->event_cap) {
        ctx->event_cap *= 2;
        ctx->events = realloc(ctx->events, sizeof(kevent_t) * ctx->event_cap);

    }
    ctx->event_count ++;
}

void event_del(event_ctx_t * ctx, int fd)
{
    struct kevent change;
    EV_SET(&change, fd, EVFILT_READ | EVFILT_WRITE | EVFILT_EXCEPT, EV_DELETE, 0, 0, NULL);
    kevent(ctx->fd, &change, 1,NULL, 0, NULL);
    ctx->event_count --;
}

int event_wait(event_ctx_t * ctx, array_t * fired_events, long long ms)
{
    struct timespec timeout;
    timeout.tv_sec = ms/1000; // 5秒
    timeout.tv_nsec = (ms%1000)*1000;
    int ready = kevent(ctx->fd, NULL, 0, ctx->events, ctx->event_cap, &timeout);
    array_resize(fired_events, ready);
    for (int i = 0; i < ready; ++ i) {
        struct kevent e = ctx->events[i];
        int events = 0; 
        if (e.filter & EVFILT_READ) events |= IO_READ; 
        if (e.filter & EVFILT_WRITE) events |= IO_WRITE; 
        if (e.filter & EVFILT_EXCEPT) events = IO_READ | IO_WRITE;
        array_set(fired_events, i, encode_event(e.ident, events));
    }
    return ready;
}
