#include <stdio.h>
#include <stdint.h>
#include <setjmp.h>

typedef struct 
{
    uint64_t rsp;
    uint64_t rbp;
    uint64_t rip;

}context_t;


typedef struct coroutine_t
{
    struct coroutine_t * pre_ctx;
    context_t * ctx;
    char * stack; 
}context_t;

extern int save_ctx(context_t * );
extern int restore_ctx(context_t * );

void co_create(context_t * ctx, long (*entry)(void *), uint64_t stack_size) 
{
}


void show(context_t t)
{
    printf("%d\n", t.rip);
}

int add(int a, int b) 
{
    return a + b;
}

int main()
{
    printf("%d\n", sizeof(context_t));
    printf("%d\n", main);
    context_t ctx;
    int res = save_ctx(&ctx);
    printf("save_ctx:%d\n", res);
    if (res == 0) {
        restore_ctx(&ctx);
    }
    show(ctx);
}
