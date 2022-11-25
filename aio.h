#pragma once

#if !defined(__APPLE__)
#include <sys/epoll.h>
#endif
#include <sys/types.h>
#include "map.h"

typedef enum 
{
    IO_READ = 1,
    IO_WRITE = 2,
}io_event_t;

typedef struct
{
    int read_co;
    int write_co;
} wait_info_t;

typedef struct io_mgr_t
{
    int epoll_id;
    int epoll_max;
    struct epoll_event * epoll_events;
    map_t wait_map;
} io_mgr_t;

void io_init();
void io_add(int fd);
void io_del(int fd);
void io_wait(int fd, int events);
void io_update(suseconds_t timeout);
void io_destroy();


