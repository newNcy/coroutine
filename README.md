# 一个c的协程实现
+ 协程带有一个自己的栈,大小为128k,通过切换上下文实现
+ 线程不安全，协程执行在单线程(没法利用多核)，协程函数间不用同步

```c
/* 
 *最简单的例子 tests/test.c
 * 两次创建协程，第一个等待协程执行完毕获取返回值
 * 第二次不等待直接往下执行
 */

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
```
