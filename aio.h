#pragma once

#include <sys/types.h>
#include "map.h"
#include "array.h"
#include "list.h"

struct co_t;
typedef struct co_t co_t;
typedef enum 
{
    IO_READ = 1,
    IO_WRITE = 2,
};

typedef struct
{
    int fd;
    int events;
}io_event_t;

typedef struct
{
    list_t * reader;
    list_t * writer;
} wait_info_t;

typedef struct event_ctx_t event_ctx_t;

typedef struct aio_mgr_t
{
    array_t * fired_events;
    map_t * wait_map;
    list_t * dead;
    event_ctx_t * event_ctx;
} aio_t;

aio_t * aio_create();
void aio_add(aio_t * aio, int fd);
void aio_del(aio_t * aio, int fd);
void aio_wait(aio_t * aio, int fd, int events);
any_t encode_event(int fd, int events);
void decode_event(any_t event, int * fd, int * events);
void io_setnoblock( int fd);
void aio_update(aio_t * aio, long long timeout);
void aio_destroy(aio_t * aio);
void aio_debug_print_info();


