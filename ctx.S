#if defined(__APPLE__)
.global  _swap_ctx
_swap_ctx:
#else
.global swap_ctx
swap_ctx:
#endif
#ifdef WIN32
    //return address
    movq (%rsp), %rax  
    movq %rsp, (%rcx)
    movq %rbp, 8(%rcx)
    movq %rax, 16(%rcx)
    movq %rdi, 24(%rcx)
    movq %rsi, 32(%rcx)
    movq %rbx, 56(%rcx)
    movq %r12, 64(%rcx)
    movq %r13, 72(%rcx)
    movq %r14, 80(%rcx)
    movq %r15, 88(%rcx)

    // restore stack and ret address
    movq (%rdx), %rsp
    movq 8(%rdx), %rbp
    movq 16(%rdx), %rax
    movq %rax, (%rsp)
    // callee save register restoring
    movq 24(%rdx), %rdi
    movq 32(%rdx), %rsi
    movq 56(%rdx), %rbx
    movq 64(%rdx), %r12
    movq 72(%rdx), %r13
    movq 80(%rdx), %r14
    movq 88(%rdx), %r15
    // put return value
    movq 96(%rdx), %rax

    // pass args when enter function
    movq 40(%rdx), %rcx
    movq 48(%rdx), %rdx
#else
    //return address
    movq (%rsp), %rax  
    movq %rsp, (%rdi)
    movq %rbp, 8(%rdi)
    movq %rax, 16(%rdi)
    movq %rbx, 56(%rdi)
    movq %r12, 64(%rdi)
    movq %r13, 72(%rdi)
    movq %r14, 80(%rdi)
    movq %r15, 88(%rdi)

    // restore stack and ret address
    movq (%rsi), %rsp
    movq 8(%rsi), %rbp
    movq 16(%rsi), %rax
    movq %rax, (%rsp)
    // callee save register restoring
    movq 56(%rsi), %rbx
    movq 64(%rsi), %r12
    movq 72(%rsi), %r13
    movq 80(%rsi), %r14
    movq 88(%rsi), %r15
    // put return value
    movq 96(%rsi), %rax

    // pass args when enter function
    movq 24(%rsi), %rdi
    movq 32(%rsi), %rsi
#endif

    ret

