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
    CO_RUNNING,
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
    char * stack;
    int last;
}co_t;


typedef struct 
{
    int inited;
    int running;
    int last;
    list_t * free_list;
    array_t * co_pool;
    heap_t * timer_mgr;
    io_mgr_t io_mgr;
} env_t;


static int CO_ID_INVALID = -1;
typedef struct 
{
    int co_id;
    env_t * env;
}awaitable_t;


void co_init();

cid_t co_create(void *entry, void * args);
void * co_resume(cid_t id);
void co_yield();

awaitable_t co_start(void * entrry, void * args);
void *co_await(awaitable_t awaitable);

int co_running();
void co_finish();
int co_is_all_finish();

void co_event_init();
void co_loop();
env_t * thread_env();
void * co_main(void * entry, void * args);

#ifdef __cplusplus
}
#endif

