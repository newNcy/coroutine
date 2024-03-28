#pragma once

typedef uint64_t reg_t;

typedef struct 
{
    reg_t rsp;
    reg_t rbp;
    reg_t rip;

    //for int arguments
    reg_t rdi;
    reg_t rsi;
    reg_t rcx;
    reg_t rdx;

    //callee save
    reg_t rbx;
    reg_t r12;
    reg_t r13;
    reg_t r14;
    reg_t r15;
    
    //for return value
    reg_t rax;

} ctx_t;


#ifdef __cplusplus
extern "C" {
#endif

void *swap_ctx(ctx_t * cur, ctx_t * next);

#ifdef __cplusplus
}
#endif
