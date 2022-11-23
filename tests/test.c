#include "coroutine.h"


int co_func()
{
    printf("sleep 1s\n");
    sleep(1);
    printf("sleep end\n");
    return 24;
}

int main()
{
    printf("main\n");
    co_main(co_func, 0);
    printf("main\n");
}
