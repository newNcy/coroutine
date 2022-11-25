#pragma once

#include <stdio.h>
#include <stdint.h>
#include "hook.h"
#include "aio.h"
#include "array.h"
#include "heap.h"
#include "timer.h"
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

typedef uint64_t reg_t;

typedef struct 
{
    reg_t rsp;
    reg_t rbp;
    reg_t rip;

    reg_t rdi;
    reg_t rsi;

    //callee save
    reg_t rbx;
    reg_t r12;
    reg_t r13;
    reg_t r14;
    reg_t r15;
    
    //for return value
    reg_t rax;

} context_t;

struct coroutine_t;

typedef void * (*coroutine_entry_t)(void *args);
typedef struct coroutine_t
{
    context_t ctx;
    context_t main;
    coroutine_entry_t entry;
    co_status_t status;

    char * stack;
    int last_id;
}coroutine_t;


typedef struct 
{
    int running;
    array_t coroutines;
} schedule_t;

typedef struct 
{
    schedule_t schedule;
    heap_t timer_mgr;
    io_mgr_t io_mgr;
    int inited;
}env_t;

static int CO_ID_INVALID = -1;
typedef struct 
{
    int co_id;
    env_t * env;
}awaitable_t;


void co_init();

int co_create(void *entry, void * args);
void * co_resume(int co);
void co_yield();

awaitable_t co_start(void * entrry, void * args);
void *co_await(awaitable_t awaitable);

int co_running();
int co_count();
void co_finish();
int co_is_all_finish();

env_t * thread_env();
void * co_main(void * entry, void * args);

#ifdef __cplusplus
}
#endif

