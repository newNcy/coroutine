#include "aio.h"
#include "macros.h"
#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/select.h>
#endif
#include <stdlib.h>

struct event_ctx_t
{
    int max_fd;
    fd_set all_fds;
};

event_ctx_t * event_create()
{
    event_ctx_t * ctx = (event_ctx_t*)malloc(sizeof(event_ctx_t));
    ctx->max_fd = 0;
    FD_ZERO(&ctx->all_fds);
    return ctx;
}

void event_destroy(event_ctx_t * ctx)
{
    free(ctx);
}

void event_add(event_ctx_t * ctx, int fd)
{
    FD_SET(fd, &ctx->all_fds);
    if (fd > ctx->max_fd) {
        ctx->max_fd = fd;
    }
}

void event_del(event_ctx_t * ctx, int fd)
{
    FD_CLR(fd, &ctx->all_fds);
    if (ctx->max_fd == fd) {
        ctx->max_fd --;
        while (!FD_ISSET(ctx->max_fd, &ctx->all_fds)) {
            ctx->max_fd --;
        }
    }
}

int event_wait(event_ctx_t * ctx, array_t * fired_events, long long ms)
{
    fd_set read_fds = ctx->all_fds;
    fd_set write_fds = ctx->all_fds;
    fd_set execpt_fds = ctx->all_fds;
    struct timeval tv;
    tv.tv_sec = ms/1000;
    tv.tv_usec = (ms%1000)*1000;
    int ready = select(ctx->max_fd + 1, &read_fds, &write_fds, &execpt_fds, ms == -1L? NULL : &tv);
    array_resize(fired_events, ready);
    if (ready > 0) {
        for (int i = 0; i<= ctx->max_fd; ++ i) {
            int events = 0;
            if (FD_ISSET(i, &read_fds)) { events |= IO_READ; }
            if (FD_ISSET(i, &write_fds)) { events |= IO_WRITE; }
            if (FD_ISSET(i, &execpt_fds)) { events = IO_READ | IO_WRITE; }
            if (events) {
                array_set(fired_events, i, encode_event(i, events));
            }
        }
    }

    return ready;
}
