#include "aio.h"
#include "macros.h"
#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/select.h>
#endif
#include <stdlib.h>

typedef struct
{
    int max_fd;
    fd_set all_fds;
}select_state_t;

void event_init(io_mgr_t * io_mgr)
{
    select_state_t * state = (select_state_t*)malloc(sizeof(select_state_t));
    state->max_fd = 0;
    FD_ZERO(&state->all_fds);
    io_mgr->event_ctx = state;
}

void event_add(io_mgr_t * io_mgr, int fd)
{
    select_state_t * state = (select_state_t*)io_mgr->event_ctx;
    FD_SET(fd, &state->all_fds);
    if (fd > state->max_fd) {
        state->max_fd = fd;
    }
}

void event_del(io_mgr_t * io_mgr, int fd)
{
    select_state_t * state = (select_state_t*)io_mgr->event_ctx;
    FD_CLR(fd, &state->all_fds);
    return;
    if (state->max_fd == fd) {
        state->max_fd --;
        while (!FD_ISSET(state->max_fd, &state->all_fds)) {
            state->max_fd --;
        }
    }
}

int event_wait(io_mgr_t * io_mgr, long long ms)
{
    select_state_t * state = (select_state_t*)io_mgr->event_ctx;
    fd_set read_fds = state->all_fds;
    fd_set write_fds = state->all_fds;
    fd_set execpt_fds = state->all_fds;
    struct timeval tv;
    tv.tv_sec = ms/1000;
    tv.tv_usec = (ms%1000)*1000;
    int ready = select(state->max_fd + 1, &read_fds, &write_fds, &execpt_fds, ms == -1L? NULL : &tv);
    array_resize(&io_mgr->fired_events, ready);
    if (ready > 0) {
        int j = 0;
        for (int i = 0; i<= state->max_fd; ++ i) {
            int events = 0;
            if (FD_ISSET(i, &read_fds)) { events |= IO_READ; }
            if (FD_ISSET(i, &write_fds)) { events |= IO_WRITE; }
            if (events) {
                array_set(&io_mgr->fired_events, j++, encode_event(i, events));
            }
        }
    }

    return ready;
}

int event_destroy(io_mgr_t * io_mgr_t)
{
    return 0;
}
