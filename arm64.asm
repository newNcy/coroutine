.global _swap_ctx
_swap_ctx:
    // Return address
    

    str lr, [x0, #16]
    str fp, [x0, #8]
    mov x9, sp
    str x9, [x0]

    mov x3, x1
    ldr fp, [x3, #8]
    ldr x9, [x3, #0]
    mov sp, x9
    ldr lr, [x3, #16]

    ldr x0, [x3, #24]
    ldr x1, [x3, #32]
    ret
