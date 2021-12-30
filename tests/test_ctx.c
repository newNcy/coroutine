#include "coroutine.h"


void foo2()
{
	printf("hello coroutine 2\n");
}

void * foo(int arg)
{
	printf("hello coroutine with arg %d\n", arg);
    return arg + 24;
}


int main(int argc, char * argv[]) 
{
    int co = co_start(foo, 12);
    printf("in main\n");
    printf("await co %d:%d\n",co, co_await(co));
	return 0;
}
