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
void co_bootstrap(coroutine_t * co, void * args)
{
    co->entry(args);
    co->status = CO_FINISH;
	co_yield();
}

void co_init()
{
    schedule.running = CO_ID_INVALID;
    schedule.count = 0;
    schedule.max = 0;
}

void co_finish()
{
	for (int i = 0 ; i < schedule.count; ++i) {
		if (schedule.coroutines[i].stack) {
			free(schedule.coroutines[i].stack);
			schedule.coroutines[i].stack = NULL;
		}
	}
    schedule.running = CO_ID_INVALID;
    schedule.count = 0;
    schedule.max = 0;
    free(schedule.coroutines);
}

int co_create(void * entry, void * args)
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

	/* handle coroutine memory */
    if (id == schedule.max) { 
		if (schedule.max == 0) {
			int max = 2;
			schedule.coroutines = (coroutine_t*)malloc(max * sizeof(coroutine_t));
			if (!schedule.coroutines) {
				return CO_ID_INVALID;
			}
			schedule.max = max;
		}else {
			int new_max = schedule.max * 2;
			void * new_mem = realloc(schedule.coroutines, new_max * sizeof(coroutine_t));
			if (!new_mem) {
				return CO_ID_INVALID;
			}
			schedule.max = new_max;
			schedule.coroutines = (coroutine_t*)new_mem;
		}

		id = schedule.count;
    }


    coroutine_t * co = &schedule.coroutines[id];

	/* uninitialized coroutine */
    if (id == schedule.count) {
        co->stack = (char*)malloc(CO_STACK_SIZE);
    }

    co->entry = (coroutine_entry_t)entry;
    co->status = CO_SUSPEND;

    co->ctx.rbp = (uint64_t)(co->stack + CO_STACK_SIZE);
    co->ctx.rsp = co->ctx.rbp - 8; //call 指令会把返回地址push到栈上，ret时弹出并跳转过去，swap_ctx里将co_bootstrap地址放到协程栈顶然后ret,所以预分配8byte
    co->ctx.rip = (uint64_t)co_bootstrap;
    co->ctx.rcx = (uint64_t)co;
    co->ctx.rdx = (uint64_t)args;

    schedule.count ++;
    return id;
}

void co_resume(int id)
{
    if (id >= 0 && id < schedule.count) {
        coroutine_t * co = &schedule.coroutines[id];
        if (co && co->status == CO_SUSPEND) {
            co->status = CO_RUNNING;
			debug("[%d] resume [%d]", schedule.running, id);
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
        if (co && (co->status == CO_RUNNING || co->status == CO_FINISH)) {
			if (co->status != CO_FINISH) {
				co->status = CO_SUSPEND;
			}
			debug("[%d] yield [%d]", schedule.running, co->last_id);
            schedule.running = co->last_id;
            swap_ctx(&co->ctx, &co->main);
        }
    }
}


int co_is_all_finish()
{
	int done = true;
	for (int id = 0; id < schedule.count; ++ id) {
		done = done && (schedule.coroutines[id].status == CO_FINISH);
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



