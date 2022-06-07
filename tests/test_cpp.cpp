#include "coroutine.h"


void co_func()
{
    printf("%s\n", __PRETTY_FUNCTION__);
}

extern "C" int main()
{
    co_start(async co_func, 0);
}
