#include <stdio.h>
#include <stdint.h>
#include <malloc.h>

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

struct coroutine_t;
typedef void * (*coroutine_entry_t)(struct coroutine_t *,void *args);

typedef enum 
{
    CO_SUSPEND,
    CO_RUNNING,
    CO_FINISH
}co_status_t;

typedef struct coroutine_t
{
    context_t ctx;
    context_t main;
    coroutine_entry_t entry;
    co_status_t status;
}coroutine_t;


extern uint64_t swap_ctx(context_t * cur, context_t * next);


void co_bootstrap(coroutine_t * co, void *args)
{
    co->entry(co, args);
    co->status = CO_FINISH;
    swap_ctx(&co->ctx, &co->main);
}


coroutine_t * co_create(coroutine_entry_t entry) 
{
    coroutine_t * co = (coroutine_t*)malloc(sizeof(coroutine_t));

    co->ctx.rbp = (uint64_t)((char*)malloc(CO_STACK_SIZE) + CO_STACK_SIZE);
    co->ctx.rsp = co->ctx.rbp;
    co->ctx.rip = (uint64_t)co_bootstrap;
    co->ctx.rcx = (uint64_t)co;

    co->entry = entry;
    co->status = CO_SUSPEND;
    return co;
}

void co_resume(coroutine_t * co)
{
    if (co->status == CO_SUSPEND) {
        co->status = CO_RUNNING;
        swap_ctx(&co->main, &co->ctx);
    }
}

void co_yield(coroutine_t * co)
{
    if (co->status == CO_RUNNING) {
        co->status = CO_SUSPEND;
        swap_ctx(&co->ctx, &co->main);
    }
}

void print_coroutine(coroutine_t * co)
{
    printf("co:rsp:%x, rbp:%x, rip:%x  main:rsp:%x, rbp:%x, rip:%x\n", co->ctx.rsp, co->ctx.rbp, co->ctx.rip, co->main.rsp, co->main.rbp,co->main.rip);
}

void num(coroutine_t* co)
{
    print_coroutine(co);
    for (int i = 0 ; i < 5; ++ i) {
        printf("loop in coroutine:%d\n", i);
        co_yield(co);
    }
    printf("end of co\n");
}

int main()
{
    coroutine_t * co = co_create((coroutine_entry_t)num);
    for (int i = 0 ; i < 5; ++ i) {
        printf("loop in main:%d\n", i);
        co_resume(co);
    }
    co_resume(co);
    //co_resume(co);
    printf("end of main\n");
    return 0;
}
