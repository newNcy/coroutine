#include <stdio.h>
#include "coroutine.h"

void foo(coroutine_t* co)
{
    printf("foo 1\n");
    co_yield(co);
    printf("foo 2\n");
}

int main()
{
    coroutine_t * co = co_create((coroutine_entry_t)foo);
    printf("main 1\n");
    co_resume(co);
    printf("main 2\n");
    co_resume(co);
    printf("main 3\n");
    co_destroy(co);
    return 0;
}
