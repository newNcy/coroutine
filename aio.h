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

typedef struct io_mgr_t
{
    void * event_ctx;
    array_t fired_events;
    map_t wait_map;
    list_t * dead;
} io_mgr_t;

void io_init();
void io_add(int fd);
void io_del(int fd);
void io_wait(int fd, int events);
any_t encode_event(int fd, int events);
void decode_event(any_t event, int * fd, int * events);
void io_setnoblock(int fd);
void io_update(long long timeout);
void io_destroy();


