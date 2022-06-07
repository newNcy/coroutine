#include "coroutine.h"
#include <unistd.h>


void co_func()
{
    printf("%s\n", __PRETTY_FUNCTION__);
    sleep(1);
    printf("%s\n", __PRETTY_FUNCTION__);
}

extern "C" int main()
{
    co_start(async co_func, 0);
}
