#include <stdio.h>
#include <stdint.h>
#include <malloc.h>

#define CO_STACK_SIZE 1024 * 100

typedef struct 
{
    uint64_t rsp;
    uint64_t rbp;
    uint64_t rip;

    uint64_t rdx;
    uint64_t r;
    
} context_t;


typedef struct coroutine_t
{
    context_t * ctx;
    context_t * main;
}coroutine_t;

typedef void * (*coroutine_entry_t)(coroutine_t *,void *args);

extern uint64_t save_ctx(context_t * );
extern uint64_t restore_ctx(context_t * );
extern uint64_t swap_ctx(context_t * cur, context_t * next);


coroutine_t * co_create(coroutine_entry_t entry) 
{
    coroutine_t * co = (coroutine_t*)malloc(sizeof(coroutine_t));
    co->ctx = (context_t*)malloc(sizeof(context_t));
    co->main= (context_t*)malloc(sizeof(context_t));

    co->ctx->rbp = (uint64_t)((char*)malloc(CO_STACK_SIZE) + CO_STACK_SIZE /2);
    co->ctx->rsp = co->ctx->rbp;
    co->ctx->rip = (uint64_t)entry;
    return co;
}

void num(coroutine_t* co)
{
    printf("co:rsp:%p, rbp:%p, rip:%p  main:rsp:%p, rbp:%p, rip:%p\n", co->ctx->rsp, co->ctx->rbp, co->ctx->rip, co->main->rsp, co->main->rbp,co->main->rip);
    fflush(stdout);
    swap_ctx(co->ctx, co->main);
    //fflush(stdout);
}

int add(int a, int b) 
{
    return a + b;
}

int main()
{
    coroutine_t * co = co_create((coroutine_entry_t)num);
    //printf("in main %p\n", co);
    co_resume(co);
    printf("co:rsp:%p, rbp:%p, rip:%p  main:rsp:%p, rbp:%p, rip:%p\n", co->ctx->rsp, co->ctx->rbp, co->ctx->rip, co->main->rsp, co->main->rbp,co->main->rip);
    fflush(stdout);
    //printf("%p %p %p\n", co->ctx->rip, co->ctx->rsp, co->ctx->rbp);
    //printf("%p %p %p\n", co->main->rip, co->main->rsp, co->main->rbp);
    printf("back\n");
    //fflush(stdout);
}
