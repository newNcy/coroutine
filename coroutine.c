#include "coroutine.h"
#include <string.h>
#include <malloc.h>

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
    int count;
    int max;
    coroutine_t * coroutines;
} schedule_t;


static schedule_t schedule;

extern uint64_t swap_ctx(context_t * cur, context_t * next);

/*
 * 需要协程函数执行完的时候改变状态
 */
void co_bootstrap(coroutine_t * co)
{
    co->entry();
    co->status = CO_FINISH;
    //printf("[%x] finish with stack:[%x:%x]\n", co, co->ctx.rbp, co->ctx.rsp);
    schedule.running = co->last_id;
    swap_ctx(&co->ctx, &co->main);
}

/*
 * max主要是需要考虑协程使用的内存
 */
void co_init(int max)
{
    schedule.running = CO_ID_INVALID;
    schedule.count = 0;
    schedule.max = max;
    schedule.coroutines = (coroutine_t*)malloc(max * sizeof(coroutine_t));
    memset(schedule.coroutines, 0 , max * sizeof(coroutine_t));
}

void co_finish()
{
    schedule.running = CO_ID_INVALID;
    schedule.count = 0;
    schedule.max = 0;
    free(schedule.coroutines);
}

int co_create(coroutine_entry_t entry)
{
    int id = schedule.count;
    if (id == schedule.max) {
        for (int i = 0; i < schedule.max; ++ i) {
            if (schedule.coroutines[i].status == CO_FINISH) {
                id = i;
                break;
            }
        }
    }
    if (id == schedule.max) { 
        return CO_ID_INVALID;
    }
    schedule.count ++;

    coroutine_t * co = &schedule.coroutines[id];

    if (!co->stack) {
        co->stack = (char*)malloc(CO_STACK_SIZE);
    }

    co->entry = entry;
    co->status = CO_SUSPEND;

    co->ctx.rbp = (uint64_t)(co->stack + CO_STACK_SIZE);
    co->ctx.rsp = co->ctx.rbp - 32; // 只知道需要8个字节放返回地址，剩下24个字节或者三个地址不知道哪里被修改了，就只好挪一挪腾出来32个字节,不然内存越界了
    co->ctx.rip = (uint64_t)co_bootstrap;
    co->ctx.rcx = (uint64_t)co;

    //printf("[%x] created with stack:[%x:%x] %x\n", co, co->ctx.rbp, co->ctx.rsp, stack);
    return id;
}

void co_resume(int id)
{
    if (id >= 0 && id < schedule.count) {
        coroutine_t * co = &schedule.coroutines[id];
        if (co && co->status == CO_SUSPEND) {
            co->status = CO_RUNNING;
            co->last_id = schedule.running;
            schedule.running = id;
            swap_ctx(&co->main, &co->ctx);
        }
    }

}

void co_yield()
{
    int id = schedule.running;
    if (id >= 0 && id < schedule.count) {
        coroutine_t * co = &schedule.coroutines[id];
        if (co && co->status == CO_RUNNING) {
            co->status = CO_SUSPEND;
            schedule.running = co->last_id;
            swap_ctx(&co->ctx, &co->main);
        }
    }
}



int co_running()
{
    return schedule.running;
}



