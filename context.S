.global  swap_ctx

swap_ctx:
#if defined(__x86_64__)
    movq (%rsp), %rax
    movq %rsp, (%rdi)
    movq %rbp, 8(%rdi)
    movq %rax, 16(%rdi)
    movq %rbx, 40(%rdi)
    movq %r12, 48(%rdi)
    movq %r13, 56(%rdi)
    movq %r14, 64(%rdi)
    movq %r15, 72(%rdi)

    // restore stack and ret address
    movq (%rsi), %rsp
    movq 8(%rsi), %rbp
    movq 16(%rsi), %rax
    movq %rax, (%rsp)
    // callee save register restoring
    movq 40(%rsi), %rbx
    movq 48(%rsi), %r12
    movq 56(%rsi), %r13
    movq 64(%rsi), %r14
    movq 72(%rsi), %r15

    // pass args when enter function
    movq 24(%rsi), %rdi
    movq 32(%rsi), %rsi
#endif
    ret

