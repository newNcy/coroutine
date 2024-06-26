#include "coroutine.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "array.h"
#include "heap.h"
#include "list.h"
#include "macros.h"


extern long long process_timer();
env_t * thread_env()
{
    static thread_local env_t env = {0};
    return &env;
}

void co_wrap(co_t * co, void * args)
{
    void * ret = co->entry(args);
    co_debug("return with %d", ret);
    co->status = CO_FINISH;
    while (!list_empty(co->wait_list)) {
        co_t * co = (co_t*)list_pop_front(co->wait_list);
        co_resume(co);
    }
    list_push_back(thread_env()->free_list, co_running());
    co_yield(ret);
}

void co_init()
{
    env_t * env = thread_env();
	if (!env->inited) {
        env->inited = 1;

        env->free_list = list_create();
        env->co_pool = array_create();
        env->timer_mgr = timer_mgr_init();
		env->running = nullptr;
        env->aio = aio_create();

        hook_sys_call();
	}
}

co_t * co_create(void * entry, void * args)
{
    env_t * env = thread_env();
    if (!env->inited) {
        co_init();
    }
    co_t * co = NULL;
    if (!list_empty(env->free_list)) {
        co = list_pop_front(env->free_list);
    }
    /* handle coroutine memory */
    if (!co) {
        co = (co_t*)malloc(sizeof(co_t));
        co->stack = (char*)malloc(CO_STACK_SIZE);
        co->wait_list = list_create();
        co->last = nullptr;
        co->id = array_size(env->co_pool);
        array_push(env->co_pool, co);
    }

    co->entry = (co_entry_t)entry;
    co->status = CO_SUSPEND;

    co->ctx.stack_base = (uint64_t)(co->stack + CO_STACK_SIZE);

    co->ctx.stack_pointer = co->ctx.stack_base - 16; 
    co->ctx.interp_pointer = (uint64_t)co_wrap;
    co->ctx.int_args[0] = (reg_t)co;
    co->ctx.int_args[1] = (uint64_t)args;

    return co;
}

void co_destroy(co_t * co)
{
    assert(co && co->stack);
    free(co->stack);
    free(co->wait_list);
    free(co);
}

void * co_resume(co_t * co)
{
    assert(co && "invalid coroutine");
    assert(co->status == CO_SUSPEND  && "coroutine must suspended");
    env_t * env = thread_env();
    co_debug("resume [%d]", co->id);
    co->last = co_running();
    env->running = co;
    return swap_ctx(&co->main, &co->ctx, 1);
}

void co_yield(void * ret)
{
    env_t * env = thread_env();
    co_t * co = co_running();
    assert(co && "can not yield in main routine");
    co_debug("%s [%d]", co->status == CO_FINISH?"finish,back to":"yield", co->last? co->last->id: -1);
    env->running = co->last;
    co->ctx.return_value = ret;
    swap_ctx(&co->ctx, &co->main, 0);
}

awaitable_t co_start(void * entry, void * args)
{
    co_t * co = co_create(entry, args);
    co_resume(co);
    awaitable_t ret;
    ret.co = co;
    ret.env = thread_env();
    return ret;
}

void *co_await(awaitable_t awaitable)
{
    assert (awaitable.env == thread_env());
    co_t * co = awaitable.co;
    env_t * env = thread_env();
    if (co) {
        if (co->status == CO_FINISH) {
            return (void*)co->main.return_value;
        } else {
            list_push_back(co->wait_list, co_running());
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
    heap_destroy(env->timer_mgr);
    aio_destroy(env->aio);
}


int co_is_all_finish()
{
    env_t * env = thread_env();
    return 0;
}

co_t * co_running()
{
    return thread_env()->running;
}

int co_count()
{
    env_t * env = thread_env();
    return array_size(env->co_pool) - list_size(env->free_list);
}

void co_loop()
{
    env_t * env = thread_env();
    while (list_size(env->free_list) != array_size(env->co_pool)) {
        long long next_wake = process_timer();
        aio_update(env->aio, next_wake);
    }
    co_finish();
}


void * co_main(void * entry, void * args)
{
    co_init();
    co_start(entry, args);
    co_loop();
}

