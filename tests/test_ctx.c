#include "coroutine.h"

void f()
{
    co_yield();
}

int main(int argc, char * argv[]) 
{
    co_init();
    int cid = co_create(f, NULL);
    uint64_t s = ns();
    printf("resume with %lld ns\n", s);
    co_resume(cid);
    uint64_t e = ns();
	return 0;
}
