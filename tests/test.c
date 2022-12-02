#include "coroutine.h"


void sleep_for_sec(int sec)
{
    sleep(sec);
    printf("%d wake up after %d seconds\n", co_running(), sec);
}

void start()
{
    for (int i = 1; i <= 5000; ++ i) {
        co_start(sleep_for_sec, 2);
    }
}

int main()
{
    printf("main\n");
    co_main(start, 0);
    printf("main\n");
}
