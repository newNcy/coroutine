#include "aio.h"
#include <sys/epoll.h>

typedef struct
{
    int max;
    int fd;
    struct epoll_event * events;
}epoll_state_t;

void event_init(io_mgr_t * io_mgr)
{
    epoll_state_t * state = (epoll_state_t*)malloc(sizeof(epoll_state_t));
    state->max = 1024;
    state->fd = epoll_create(1);
    state->events = (struct epoll_event*)malloc(state->max * sizeof(struct epoll_event));
    memset(state->events, 0, state->max * sizeof(struct epoll_event));
    io_mgr->event_ctx = state;
}

void event_add(io_mgr_t * io_mgr, int fd)
{
    epoll_state_t * state = (epoll_state_t*)io_mgr->event_ctx;
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLOUT;
    epoll_ctl(state->fd, EPOLL_CTL_ADD, fd, &event);
}

void event_del(io_mgr_t * io_mgr, int fd)
{
    epoll_state_t * state = (epoll_state_t*)io_mgr->event_ctx;
    epoll_ctl(state->fd, EPOLL_CTL_DEL, fd, NULL);
}

int event_wait(io_mgr_t * io_mgr, long long timeout)
{
    epoll_state_t * state = (epoll_state_t*)io_mgr->event_ctx;
    int ready = epoll_wait(state->fd, state->events, state->max, timeout);
    array_resize(&io_mgr->fired_events, ready);
    for (int i = 0; i < ready; ++ i) {
        struct epoll_event * e = &state->events[i]; 
        int events = 0;
        if (e->events & EPOLLIN) { events |= IO_READ;}
        if (e->events & EPOLLOUT) { events |= IO_WRITE;}
        array_set(&io_mgr->fired_events, i, encode_event(e->data.fd, events));
    }
    return ready;
}

void event_destroy(io_mgr_t * io_mgr)
{
}
