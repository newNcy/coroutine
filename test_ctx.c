#include "coroutine.h"


void foo()
{
	printf("hello coroutine\n");
}


int main(int argc, char * argv[]) 
{
	co_init();
	co_start(foo, 0);
	return 0;
}
