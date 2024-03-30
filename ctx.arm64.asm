
.align 2
.global _swap_ctx
_swap_ctx:
    
    str lr, [x0, #0]    // cur->interp_pointer = lr(x30)
    str fp, [x0, #8]    // cur->stack_base = fp

    mov x9, sp
    str x9, [x0, #16]   // cur->stack_pointer = sp

    ldr lr, [x1, #0]    // lr = next->interp_pointer
    ldr fp, [x1, #8]    // fp = next->stack_base
    ldr x9, [x1, #16]  
    mov sp, x9          // sp = next->stack_pointer

    //save callee saved 
    str x19, [x0, #48]
    str x20, [x0, #56]
    str x21, [x0, #64]
    str x22, [x0, #72]
    str x23, [x0, #80]
    str x24, [x0, #88]
    str x25, [x0, #96]
    str x26, [x0, #102]
    str x27, [x0, #110]
    str x28, [x0, #118]

    //restore
    ldr x19, [x1, #48]
    ldr x20, [x1, #56]
    ldr x21, [x1, #64]
    ldr x22, [x1, #72]
    ldr x23, [x1, #80]
    ldr x24, [x1, #88]
    ldr x25, [x1, #96]
    ldr x26, [x1, #102]
    ldr x27, [x1, #110]
    ldr x28, [x1, #118]

    cmp x2, #0
    b.eq set_ret        // if in_or_out != 0 
    ldr x0, [x1, #32]   // x0 = next->int_args[0]
    ldr x1, [x1, #40]   // x1 = next->int_arg
    b done              // else
set_ret:
    ldr x0, [x0, #24]   // x0 = cur->return_value
done:
    ret
