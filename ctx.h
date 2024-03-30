#pragma once

typedef unsigned long reg_t;

typedef struct 
{
    reg_t interp_pointer;   //#0
    reg_t stack_base;       //#8
    reg_t stack_pointer;    //#16

    reg_t return_value;     //#24
    reg_t int_args[2];      //#32 40

    reg_t callee_save[CALLEE_SAVE_COUNT]; //#48
} ctx_t;


#ifdef __cplusplus
extern "C" {
#endif

void *swap_ctx(ctx_t * cur, ctx_t * nxt, int in_or_out);

#ifdef __cplusplus
}
#endif
