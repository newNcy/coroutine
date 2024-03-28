#include "coroutine.h"
#include <stdio.h>

uint64_t s = 0;
void f(int i)
{
    int local = i + 3;
    printf("i: %d local:%d\n", i, local);
    co_yield();
    printf("i: %d local:%d\n", i, local);
}

int main(int argc, char * argv[]) 
{
    co_t * co = co_create(f, 3);
    s = ns();
    co_resume(co);
    printf("resume with %lld ns\n", ns() -s);
    co_resume(co);
	return 0;
}
