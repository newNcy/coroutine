#pragma once

#if !defined(__APPLE__)
#include <sys/epoll.h>
#endif
#include <sys/types.h>
#include "map.h"

typedef struct
{
    int read_co;
    int write_co;
} wait_info_t;

typedef struct aio_mgr_t
{
    int epoll_id;
    int epoll_max;
    struct epoll_event * epoll_events;
    map_t wait_map;
} aio_mgr_t;

void aio_init();
void aio_update(suseconds_t timeout);
void aio_destroy();


