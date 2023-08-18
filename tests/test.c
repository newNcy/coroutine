#include "coroutine.h"


uint64_t s = 0;
void sleep_for_sec(int sec)
{
    sleep(sec * 1000);
    printf("%d wake up after %d seconds\n", co_running()->id, sec);
}

void start()
{
    for (int i = 1; i <= 50; ++ i) {
        co_start(sleep_for_sec, 2);
    }
}

void test_swap()
{
    uint64_t end = ns();
    printf("use %lld ns\n", end - s);
}

int main()
{
    s = ns();
    uint64_t s2 = ns();
    printf("%d\n", s2-s);
    co_main(test_swap, 0);
}
