#include "coroutine.h"
#include <stdio.h>

int f(int v)
{
    for (int i = 0; i < 10; ++ i) {
        v ++;
        co_yield(v);
    }

    return ++v;
}

int main(int argc, char * argv[]) 
{
    co_t * co = co_create(f, 3);
    for (int i = 0; i < 10; ++ i) {
        int t = co_resume(co);
        printf("t=%d\n", t);
    }
    int t = co_resume(co);
    printf("ret=%d\n", t);
	return 0;
}
