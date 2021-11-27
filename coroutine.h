#pragma once

#include <stdio.h>
#include <stdint.h>
#define CO_STACK_SIZE 1024 * 100

typedef enum 
{
    CO_SUSPEND,
    CO_RUNNING,
    CO_FINISH
}co_status_t;

static int CO_ID_INVALID = -1;

typedef void * (*coroutine_entry_t)();

void co_init(int max);
int co_create(coroutine_entry_t entry);
void co_resume(int co);
void co_yield();
int co_running();
void co_finish();


