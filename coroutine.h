#pragma once

#include <stdio.h>
#include <stdint.h>
#define CO_STACK_SIZE 1024 * 100

typedef uint64_t reg_t;

typedef struct 
{
    reg_t rsp;
    reg_t rbp;
    reg_t rip;

    reg_t rcx;
    reg_t rdx;

    //callee save
    reg_t rbx;
    reg_t r12;
    reg_t r13;
    reg_t r14;
    reg_t r15;
} context_t;


typedef enum 
{
    CO_SUSPEND,
    CO_RUNNING,
    CO_FINISH
}co_status_t;

struct coroutine_t;
typedef void * (*coroutine_entry_t)(struct coroutine_t *,void *args);

typedef struct coroutine_t
{
    context_t ctx;
    context_t main;
    coroutine_entry_t entry;
    co_status_t status;

    char * stack;
}coroutine_t;

coroutine_t * co_create(coroutine_entry_t entry);
void co_resume(coroutine_t * co);
void co_yield(coroutine_t * co);
void co_destroy(coroutine_t *co);


