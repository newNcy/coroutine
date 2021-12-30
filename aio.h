#pragma once

#if !defined(__APPLE__)
#include <sys/epoll.h>
#endif
#include "map.h"

typedef struct
{
    int read_co;
    int write_co;
} wait_info_t;

typedef struct co_io_mgr_t
{
    int epoll_id;
    int epoll_max;
    struct epoll_event * epoll_events;
    map_t wait_map;
} co_io_mgr_t;


