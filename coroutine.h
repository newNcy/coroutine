#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "hook.h"
#include "aio.h"
#include "array.h"
#include "heap.h"
#include "timer.h"
#include "list.h"
#include "ctx.h"
#include "macros.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef enum 
{
    CO_SUSPEND,
    CO_FINISH
}co_status_t;

struct co_t;
typedef int cid_t;

typedef void * (*co_entry_t)(void *args);
typedef struct co_t
{
    ctx_t ctx;
    ctx_t main;
    co_entry_t entry;
    co_status_t status;
    list_t * wait_list;
    int id;
    char * stack;
    struct co_t * last;
}co_t;


typedef struct 
{
    int inited;
    co_t * running;
    list_t * free_list;
    array_t * co_pool;
    heap_t * timer_mgr;
    io_mgr_t io_mgr;
} env_t;


static int CO_ID_INVALID = -1;
typedef struct 
{
    co_t * co;
    env_t * env;
}awaitable_t;


void co_init();

co_t * co_create(void *entry, void * args);
inline void * co_resume(co_t * id);
inline void co_yield();

awaitable_t co_start(void * entrry, void * args);
void *co_await(awaitable_t awaitable);

co_t * co_running();
void co_finish();
void co_loop();

inline env_t * thread_env();
void * co_main(void * entry, void * args);

#ifdef __cplusplus
}
#endif

