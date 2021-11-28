#pragma once

#include <stdio.h>
#include <stdint.h>
#define CO_STACK_SIZE 1024 * 128
#define true 1

typedef enum 
{
    CO_SUSPEND,
    CO_RUNNING,
    CO_FINISH
}co_status_t;

static int CO_ID_INVALID = -1;

typedef void * (*coroutine_entry_t)(void *args);

void co_init();
int co_create(void *entry, void * args);
void co_resume(int co);
void co_yield();
int co_running();
void co_finish();
int co_is_all_finish();

void co_event_init();
void co_event_loop();




