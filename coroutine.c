#include "coroutine.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "array.h"
#include "heap.h"
#include "macros.h"
thread_local env_t env;

env_t * thread_env()
{
    return &env;
}

extern uint64_t swap_ctx(context_t * cur, context_t * next);

/*
 * 需要协程函数执行完的时候改变状态
 */
void co_wrap(coroutine_t * co, void * args)
{
    void * ret = co->entry(args);
    co_debug("return with %d", ret);
    co->status = CO_FINISH;
    co->main.rax = ret;
    co_yield();
}

void co_init()
{
    env_t * env = thread_env();
	if (!env->inited) {
		thread_env()->schedule.running = CO_ID_INVALID;
		array_init(&thread_env()->schedule.coroutines);
        co_event_init();
        hook_sys_call();
        env->inited = 1;
	}
}

void co_finish()
{
    for (int i = 0 ; i < thread_env()->schedule.coroutines.size; ++i) {
        coroutine_t * co = array_get(&thread_env()->schedule.coroutines, i);
        if (co) {
            if (co->stack) {
                free(co->stack);
                co->stack = NULL;
            }
            free(co);
            array_set(&thread_env()->schedule.coroutines, i, (any_t)NULL);
        }
    }
    thread_env()->schedule.running = CO_ID_INVALID;
    array_destroy(&thread_env()->schedule.coroutines);
}

int co_create(void * entry, void * args)
{
    int id = CO_ID_INVALID;
    coroutine_t * co = NULL;
    for (int i = 0; i < thread_env()->schedule.coroutines.size; ++ i) {
        coroutine_t * exist = array_get(&thread_env()->schedule.coroutines, i);
        if (exist->status == CO_FINISH) {
            id = i;
            co = exist;
            break;
        }
    }

    /* handle coroutine memory */
    if (id == CO_ID_INVALID) {
        co = (coroutine_t*)malloc(sizeof(coroutine_t));
        co->stack = (char*)malloc(CO_STACK_SIZE);
        array_push_back(&thread_env()->schedule.coroutines, (any_t)co);
        id = thread_env()->schedule.coroutines.size - 1;
    }

    co->entry = (coroutine_entry_t)entry;
    co->status = CO_SUSPEND;

    co->ctx.rbp = (uint64_t)(co->stack + CO_STACK_SIZE);

#if defined(__APPLE__)
	// 对齐
    co->ctx.rsp = co->ctx.rbp - 16; //call 指令会把返回地址push到栈上，ret时弹出并跳转过去，swap_ctx里将co_bootstrap地址放到协程栈顶然后ret,所以预分配8byte
#else 
    co->ctx.rsp = co->ctx.rbp ; //call 指令会把返回地址push到栈上，ret时弹出并跳转过去，swap_ctx里将co_bootstrap地址放到协程栈顶然后ret,所以预分配8byte 
#endif
    co->ctx.rip = (uint64_t)co_wrap;
#ifdef WIN32
    co->ctx.rcx = (uint64_t)co;
    co->ctx.rdx = (uint64_t)args;
#else
    co->ctx.rdi = (uint64_t)co;
    co->ctx.rsi = (uint64_t)args;
#endif

    return id;
}

void * co_resume(int id)
{
    if (id >= 0 && id < thread_env()->schedule.coroutines.size) {
        coroutine_t * co = array_get(&thread_env()->schedule.coroutines, id);
        if (co && co->status == CO_SUSPEND) {
            co->status = CO_RUNNING;
            co_debug("resume [%d]", id);
            co->last_id = thread_env()->schedule.running;
            thread_env()->schedule.running = id;
            return swap_ctx(&co->main, &co->ctx);
        }
    }
    return 0;
}

void co_yield()
{
    int id = thread_env()->schedule.running;
    if (id >= 0 && id < thread_env()->schedule.coroutines.size) {
        coroutine_t * co = array_get(&thread_env()->schedule.coroutines, id);
        if (co && (co->status == CO_RUNNING || co->status == CO_FINISH)) {
            if (co->status != CO_FINISH) {
                co->status = CO_SUSPEND;
            }
            co_debug("%s [%d]", co->status == CO_FINISH?"finish,back to":"yield", co->last_id);
            thread_env()->schedule.running = co->last_id;
            swap_ctx(&co->ctx, &co->main);
        }
    }
}

awaitable_t co_start(void * entry, void * args)
{
    assert(thread_env()->inited && "co env need to be inited first");
    int co = co_create(entry, args);
    co_resume(co);
    awaitable_t ret;
    ret.co_id = co;
    ret.env = thread_env();
    return ret;
}

void *co_await(awaitable_t awaitable)
{
    assert (awaitable.env == thread_env());
    int id = awaitable.co_id;
    if (id >= 0 && id < thread_env()->schedule.coroutines.size) {
        coroutine_t * co = array_get(&thread_env()->schedule.coroutines, id);
        if (co) {
            while (co->status != CO_FINISH) {
                usleep(0);
            }
            return co->main.rax;
        }
    }
    return 0;
}



int co_is_all_finish()
{
    int done = true;
    for (int id = 0; id < thread_env()->schedule.coroutines.size; ++ id) {
        coroutine_t * co = array_get(&thread_env()->schedule.coroutines, id);
        done = done && (co->status == CO_FINISH);
        if (!done) {
            break;
        }
    }
    return done;
}

int co_running()
{
    return thread_env()->schedule.running;
}

void co_event_init()
{
    timer_mgr_init(&thread_env()->timer_mgr);
    io_init();
}

void co_event_loop()
{
    while (1) {
        long long next_wake = process_timer();
        io_update(next_wake);
        if (co_is_all_finish()) {
            break;
        }
    }
    heap_destroy(&thread_env()->timer_mgr);
}


void * co_main(void * entry, void * args)
{
    co_init();
    co_start(entry, args);
    co_event_loop();
    co_finish();
}

