#pragma once

typedef uint64_t reg_t;

typedef struct 
{
    reg_t stack_pointer;
    reg_t stack_base;
    reg_t interp_pointer;

    reg_t int_args[4];
    reg_t callee_save[4];
    reg_t return_value;

} ctx_t;


#ifdef __cplusplus
extern "C" {
#endif

void *swap_ctx(ctx_t * cur, ctx_t * next);

#ifdef __cplusplus
}
#endif
