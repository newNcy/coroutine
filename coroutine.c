#include "coroutine.h"
#include <string.h>
#include <malloc.h>

extern uint64_t swap_ctx(context_t * cur, context_t * next);

/*
 * 需要协程函数执行完的时候改变状态
 */
void co_bootstrap(coroutine_t * co, void *args)
{
    co->entry(co, args);
    co->status = CO_FINISH;
    //printf("[%x] finish with stack:[%x:%x]\n", co, co->ctx.rbp, co->ctx.rsp);
    swap_ctx(&co->ctx, &co->main);
}

coroutine_t * co_create(coroutine_entry_t entry)
{
    coroutine_t * co = (coroutine_t*)malloc(sizeof(coroutine_t));
    memset(co, 0, sizeof(coroutine_t));

    char * stack = (char*)malloc(CO_STACK_SIZE);
    co->entry = entry;
    co->status = CO_SUSPEND;
    co->stack = stack;

    memset(stack, 0, CO_STACK_SIZE);
    co->ctx.rbp = (uint64_t)(stack + CO_STACK_SIZE);
    co->ctx.rsp = co->ctx.rbp - 32; // 只知道需要8个字节放返回地址，剩下24个字节或者三个地址不知道哪里被修改了，就只好挪一挪腾出来32个字节,不然内存越界了
    co->ctx.rip = (uint64_t)co_bootstrap;
    co->ctx.rcx = (uint64_t)co;

    //printf("[%x] created with stack:[%x:%x] %x\n", co, co->ctx.rbp, co->ctx.rsp, stack);
    return co;
}

void co_destroy(coroutine_t *co)
{
    free(co->stack);
    free(co);
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


