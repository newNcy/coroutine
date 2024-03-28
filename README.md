# 一个c的协程实现
+ 协程使用2m虚拟内存作为栈
+ 每个线程自带一个调度器，不能跨线程唤醒协程

## 简单使用,不使用调度器

```c

#include "coroutine.h"

void  func()
{
    printf("func 1\n");
    co_yield();
    printf("func 2\n");
}

int main()
{
    // 初始化本线程的环境
    co_init();

    printf("main 1\n");
    co_t * co = co_create(func, NULL);
    co_resume(co);
    printf("main 2\n");
    co_resume(co);
    // 释放本线程环境
    co_finish();
}
```

## 使用调度器
```c

#include "coroutine.h"

void  func()
{
    printf("func 1\n");
    asleep(1000);
    printf("func 2\n");
}

int main()
{
   co_main(func, NULL);
}
```
