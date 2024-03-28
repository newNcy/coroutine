#include "coroutine.h"

uint64_t s = 0;
void f()
{
    uint64_t e = ns();
    printf("resume with %lld ns\n", e-s);
    co_yield();
}

int main(int argc, char * argv[]) 
{
    co_init();
    co_t * co = co_create(f, NULL);
    s = ns();
    uint64_t e = ns();
    printf("resume with %lld ns\n", e-s);
    co_resume(co);
	return 0;
}
