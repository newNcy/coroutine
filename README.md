# 一个c的协程实现
+ 协程使用2m虚拟内存作为栈
+ 每个线程自带一个调度器，不能跨线程唤醒协程

## 平台
- arm64
- x86_64

## 简单使用,不使用调度器

```c

#include "coroutine.h"
#include <stdio.h>

uint64_t s = 0;
void f(int i)
{
    int local = i + 3;
    printf("i: %d local:%d\n", i, local);
    co_yield();
    printf("i: %d local:%d\n", i, local);
}

int main(int argc, char * argv[])
{
    co_t * co = co_create(f, 3);
    s = ns();
    co_resume(co);
    printf("resume with %lld ns\n", ns() -s);
    co_resume(co);
	return 0;
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
