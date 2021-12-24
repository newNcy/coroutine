#include "coroutine.h"


void foo2()
{
	printf("hello coroutine 2\n");
}

void foo()
{
	co_start(foo2, 0);
	printf("hello coroutine\n");
}


int main(int argc, char * argv[]) 
{
	co_start(foo, 0);
	return 0;
}
