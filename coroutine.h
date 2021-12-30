#pragma once

#include <stdio.h>
#include <stdint.h>
#define CO_STACK_SIZE 1024 * 128
#define true 1

#ifdef LOG_DEBUG
#define co_debug(fmt,...) printf("[debug] [%d] "fmt"\n", co_running(),  ##__VA_ARGS__)
#else
#define co_debug
#endif

#ifdef LOG_INFO
#define co_info(fmt,...) printf("[info] [%d] "fmt"\n", co_running(),  ##__VA_ARGS__)
#else
#define co_info
#endif



typedef enum 
{
    CO_SUSPEND,
    CO_RUNNING,
    CO_FINISH
}co_status_t;

typedef struct promise_t
{
    int co_id;
    void * value;
}promise_t;

static int CO_ID_INVALID = -1;

typedef void * (*coroutine_entry_t)(void *args);

void co_init();

int co_create(void *entry, void * args);
void * co_resume(int co);
void co_yield();

int co_start(void * entrry, void * args);
void *co_await(int id);

int co_running();
int co_count();
void co_finish();
int co_is_all_finish();

void co_event_init();
void co_event_loop();




