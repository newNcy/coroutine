#pragma once

#include <stdio.h>
#include <stdint.h>
#include "hook.h"
#include "aio.h"
#include "array.h"
#include "heap.h"
#include "list.h"
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

    //for int arguments
    reg_t rdi;
    reg_t rsi;
    reg_t rcx;
    reg_t rdx;

    //callee save
    reg_t rbx;
    reg_t r12;
    reg_t r13;
    reg_t r14;
    reg_t r15;
    
    //for return value
    reg_t rax;

} ctx_t;

struct co_t;

typedef void * (*co_entry_t)(void *args);
typedef struct co_t
{
    ctx_t ctx;
    ctx_t main;
    coroutine_entry_t entry;
    co_status_t status;

    char * stack;
    int last_id;
}co_t;


typedef struct 
{
    int inited;
    int next_id;
    co_t * running;
    list_t * co_list;
    list_t * free_list;
    heap_t timer_mgr;
    io_mgr_t io_mgr;
} env_t;


static int CO_ID_INVALID = -1;
typedef struct 
{
    int co_id;
    env_t * env;
}awaitable_t;


void co_init();

co_t * co_create(void *entry, void * args);
void * co_resume(co_t * co);
void co_yield();

awaitable_t co_start(void * entrry, void * args);
void *co_await(awaitable_t awaitable);

int co_running();
int co_count();
void co_finish();
int co_is_all_finish();

void co_event_init();
void co_event_loop();
env_t * thread_env();
void * co_main(void * entry, void * args);

#ifdef __cplusplus
}
#endif

