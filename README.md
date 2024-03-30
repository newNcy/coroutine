# 一个c的协程实现
+ 协程使用2m虚拟内存作为栈
+ 每个线程自带一个调度器，不能跨线程唤醒协程

## 平台
- arm64
- x86_64
## 原理
切换协程类似函数的调用过程
### 代码跳转
通过设置ip寄存器实现代码跳转，
### 栈帧切换
设置栈底寄存器和栈顶寄存器实现栈帧切换。
### 保存被调用者保存寄存器（callee save）
我们还需要保存被调用者保存的寄存器，因为对于调用者来说这些是非易失性的，它的代码可能存在从协程切换回来后继续使用该寄存器值的行为，如果不帮它保存会造成错误。
### 传递参数
只需要关心整数参数即可，因为协程函数只接受一个参数
### 返回值
协程函数返回值赋值给返回值寄存器

### 上下文结构
上下文包含上面所说的ip，堆栈寄存器，整数参数寄存器，返回值寄存器，被调用者保存寄存器

```C
typedef unsigned long reg_t;

typedef struct 
{
    reg_t stack_pointer;
    reg_t stack_base;
    reg_t interp_pointer;
    reg_t return_value;
    reg_t int_args[2];
    reg_t callee_save[];
} ctx_t;


```

## 样例
### 不使用调度器

```c
#include "coroutine.h"
#include <stdio.h>

int f(int v)
{
    for (int i = 0; i < 10; ++ i) {
        v ++;
        co_yield(v);
    }

    return ++v;
}

int main(int argc, char * argv[]) 
{
    co_t * co = co_create(f, 3);
    for (int i = 0; i < 10; ++ i) {
        int t = co_resume(co);
        printf("t=%d\n", t);
    }
    int t = co_resume(co);
    printf("ret=%d\n", t);
	return 0;
}

```


### 使用调度器
使用调度器会在co_main里调用co_loop, co_loop会运行事件和定时器队列，唤醒等待定时器和等待io的协程
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

### 其他例子
