#include "coroutine.h"


int co_func()
{
    printf("%s sleep 1s\n", __PRETTY_FUNCTION__);
    sleep(1);
    printf("%s\n", __PRETTY_FUNCTION__);
    return 24;
}

int main()
{
    int ret =co_await(co_start(async co_func, 0));
    printf("%s %d\n", __PRETTY_FUNCTION__, ret);
    co_start(async co_func, 0);
    printf("%s\n", __PRETTY_FUNCTION__);

}
