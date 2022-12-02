#include "coroutine.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "array.h"
#include "heap.h"
#include "list.h"
#include "macros.h"
thread_local env_t env;

env_t * thread_env()
{
    return &env;
}


/*
 * 需要协程函数执行完的时候改变状态
 */
void co_wrap(co_t * co, void * args)
{
    void * ret = co->entry(args);
    co_debug("return with %d", ret);
    co->status = CO_FINISH;
    co->main.rax = ret;
    while (!list_empty(co->wait_list)) {
        cid_t id = (cid_t)list_pop_front(co->wait_list);
        co_resume(id);
    }

    co_yield();
}

void co_init()
{
    env_t * env = thread_env();
	if (!env->inited) {
        env->free_list = list_create();
        env->co_pool = array_create();
		env->running = CO_ID_INVALID;
		env->last = CO_ID_INVALID;
        env->inited = 1;

        co_event_init();
        hook_sys_call();
	}
}

cid_t co_create(void * entry, void * args)
{
    env_t * env = thread_env();
    co_t * co = NULL;
    int id = CO_ID_INVALID;
    if (!list_empty(env->free_list)) {
        id = (int)list_pop_front(env->free_list);
        co = (co_t*)array_get(env->co_pool, id);
    }
    /* handle coroutine memory */
    if (!co) {
        co = (co_t*)malloc(sizeof(co_t));
        co->stack = (char*)malloc(CO_STACK_SIZE);
        id = array_size(env->co_pool);
        co->wait_list = list_create();
        array_push(env->co_pool, co);
    }

    co->entry = (co_entry_t)entry;
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

void co_destroy(co_t * co)
{
    assert(co && co->stack);
    free(co->stack);
    free(co->wait_list);
    free(co);
}

void * co_resume(cid_t cid)
{
    env_t * env = thread_env();
    co_t * co = array_get(env->co_pool, cid);
    if (co && co->status == CO_SUSPEND) {
        co->status = CO_RUNNING;
        co_debug("resume [%d]", cid);
        co->last = env->running;
        env->running = cid;
        return swap_ctx(&co->main, &co->ctx);
    }
    return 0;
}

void co_yield()
{
    env_t * env = thread_env();
    if (env->running != CO_ID_INVALID) {
        co_t * co = (co_t*)array_get(env->co_pool, env->running);
        if (co && (co->status == CO_RUNNING || co->status == CO_FINISH)) {
            if (co->status != CO_FINISH) {
                co->status = CO_SUSPEND;
            }
            co_debug("%s [%d]", co->status == CO_FINISH?"finish,back to":"yield", co->last);
            env->running = co->last;
            swap_ctx(&co->ctx, &co->main);
        }
    }
}

awaitable_t co_start(void * entry, void * args)
{
    assert(thread_env()->inited && "co env need to be inited first");
    cid_t co = co_create(entry, args);
    co_resume(co);
    awaitable_t ret;
    ret.co_id = co;
    ret.env = thread_env();
    return ret;
}

void *co_await(awaitable_t awaitable)
{
    assert (awaitable.env == thread_env());
    cid_t id = awaitable.co_id;
    env_t * env = thread_env();
    if (id != CO_ID_INVALID) {
        co_t * co = array_get(&env->co_pool, id);
        if (co) {
            if (co->status == CO_FINISH) {
                return co->main.rax;
            } else {
                list_push_back(co->wait_list, co_running());
            }
        }
    }
    return 0;
}


void co_finish()
{
    env_t * env = thread_env();
    for (int i = 0; i < array_size(env->co_pool); ++ i) {
        co_t * co = (co_t*)array_get(env->co_pool, i);
        co_destroy(co);
    }
    array_destroy(env->co_pool);
    list_destroy(env->free_list);
    heap_destroy(&env->timer_mgr);
    io_destroy(&env->io_mgr);
}


int co_is_all_finish()
{
    env_t * env = thread_env();
    return list_size(env->free_list) == array_size(env->co_pool);
}

int co_running()
{
    return thread_env()->running;
}

void co_event_init()
{
    timer_mgr_init(&thread_env()->timer_mgr);
    io_init();
}

void co_event_loop()
{
    while (!co_is_all_finish()) {
        long long next_wake = process_timer();
        io_update(next_wake);
    }
    co_finish();
}


void * co_main(void * entry, void * args)
{
    co_init();
    co_start(entry, args);
    co_event_loop();
}

