#include "coroutine.h"
#include <string.h>
#include <stdlib.h>
#include "array.h"

typedef uint64_t reg_t;

typedef struct 
{
    reg_t rsp;
    reg_t rbp;
    reg_t rip;

    reg_t rdi;
    reg_t rsi;

    //callee save
    reg_t rbx;
    reg_t r12;
    reg_t r13;
    reg_t r14;
    reg_t r15;
    
    //for return value
    reg_t rax;

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
    array_t coroutines;
} schedule_t;

typedef struct
{
    int co_id;
    int resolved;
    int error;
};


static schedule_t schedule;

extern uint64_t swap_ctx(context_t * cur, context_t * next);

/*
 * 需要协程函数执行完的时候改变状态
 */
void co_bootstrap(coroutine_t * co, void * args)
{
    void * ret = co->entry(args);
    co_debug("return with %d", ret);
    co->status = CO_FINISH;
    co->main.rax = ret;
    co_yield();
}

void co_init()
{
	static int inited = 0;
	if (!inited) {
		schedule.running = CO_ID_INVALID;
		array_init(&schedule.coroutines);
		atexit(co_finish);
        inited = 1;
	}
}

void co_finish()
{
    for (int i = 0 ; i < schedule.coroutines.size; ++i) {
        coroutine_t * co = array_get(&schedule.coroutines, i);
        if (co) {
            if (co->stack) {
                free(co->stack);
                co->stack = NULL;
            }
            free(co);
            array_set(&schedule.coroutines, i, (any_t)NULL);
        }
    }
    schedule.running = CO_ID_INVALID;
    array_destroy(&schedule.coroutines);
}

int co_create(void * entry, void * args)
{
    int id = CO_ID_INVALID;
    coroutine_t * co = NULL;
    for (int i = 0; i < schedule.coroutines.size; ++ i) {
        coroutine_t * exist = array_get(&schedule.coroutines, i);
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
        array_push_back(&schedule.coroutines, (any_t)co);
        id = schedule.coroutines.size - 1;
    }

    co->entry = (coroutine_entry_t)entry;
    co->status = CO_SUSPEND;

    co->ctx.rbp = (uint64_t)(co->stack + CO_STACK_SIZE);

#if defined(__APPLE__)
	// 对齐
    co->ctx.rsp = co->ctx.rbp - 16; //call 指令会把返回地址push到栈上，ret时弹出并跳转过去，swap_ctx里将co_bootstrap地址放到协程栈顶然后ret,所以预分配8byte
#else 
    co->ctx.rsp = co->ctx.rbp - 8; //call 指令会把返回地址push到栈上，ret时弹出并跳转过去，swap_ctx里将co_bootstrap地址放到协程栈顶然后ret,所以预分配8byte 
#endif
    co->ctx.rip = (uint64_t)co_bootstrap;
    co->ctx.rdi = (uint64_t)co;
    co->ctx.rsi = (uint64_t)args;

    return id;
}

void * co_resume(int id)
{
    if (id >= 0 && id < schedule.coroutines.size) {
        coroutine_t * co = array_get(&schedule.coroutines, id);
        if (co && co->status == CO_SUSPEND) {
            co->status = CO_RUNNING;
            co_debug("resume [%d]", id);
            co->last_id = schedule.running;
            schedule.running = id;
            return swap_ctx(&co->main, &co->ctx);
        }
    }
    return 0;
}

void co_yield()
{
    int id = schedule.running;
    if (id >= 0 && id < schedule.coroutines.size) {
        coroutine_t * co = array_get(&schedule.coroutines, id);
        if (co && (co->status == CO_RUNNING || co->status == CO_FINISH)) {
            if (co->status != CO_FINISH) {
                co->status = CO_SUSPEND;
            }
            co_debug("%s [%d]", co->status == CO_FINISH?"finish,back to":"yield", co->last_id);
            schedule.running = co->last_id;
            swap_ctx(&co->ctx, &co->main);
        }
    }
}

int co_start(void * entry, void * args)
{
	co_init();
    int co = co_create(entry, args);
    co_resume(co);
    return co;
}

void *co_await(int id)
{
    if (id >= 0 && id < schedule.coroutines.size) {
        coroutine_t * co = array_get(&schedule.coroutines, id);
        if (co) {
            while (co->status != CO_FINISH) {
                usleep(0);
            }
            //当协程函数跑完,返回值会被放进返回值对应的寄存器，如x86的rax
            return co->main.rax;
        }
    }
    return 0;
}



int co_is_all_finish()
{
    int done = true;
    for (int id = 0; id < schedule.coroutines.size; ++ id) {
        coroutine_t * co = array_get(&schedule.coroutines, id);
        done = done && (co->status == CO_FINISH);
        if (!done) {
            break;
        }
    }
    return done;
}

int co_running()
{
    return schedule.running;
}

int co_count()
{
}

#undef main
#ifdef __cplusplus
extern "C" int co_main();
#else 
extern int co_main();
#endif

int main(int argc, char * argv)
{
    co_init();
    co_event_init();

    co_start(co_main, 0);
    co_event_loop();
    printf("done\n");
    co_finish();
    return 0;
}


