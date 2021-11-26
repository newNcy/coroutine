.global  swap_ctx , co_resume, co_yield

#input 
#
#if defined(__x86_64__)
swap_ctx:
    movq (%rsp), %rax
    movq %rsp, (%rcx)
    movq %rbp, 8(%rcx)
    movq %rax, 16(%rcx)
    movq %rbx, 40(%rcx)
    movq %r12, 48(%rcx)
    movq %r13, 56(%rcx)
    movq %r14, 64(%rcx)
    movq %r15, 72(%rcx)

    // restore stack and ret address
    movq (%rdx), %rsp
    movq 8(%rdx), %rbp
    movq 16(%rdx), %rax
    movq %rax, (%rsp)

    // callee save register restoring
    movq 40(%rdx), %rbx
    movq 48(%rdx), %r12
    movq 56(%rdx), %r13
    movq 64(%rdx), %r14
    movq 72(%rdx), %r15

    // pass args when enter function
    movq 24(%rdx), %rcx
    movq 32(%rdx), %rdx

    ret

#endif
