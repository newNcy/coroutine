.global  swap_ctx , co_resume, co_yield

#input 
#
#if defined(__x86_64__)
swap_ctx:
    movq (%rsp), %rax
    movq %rax, 16(%rcx)
    movq %rbp, 8(%rcx)
    movq %rsp, (%rcx)

    movq (%rdx), %rsp
    movq 8(%rdx), %rbp
    movq 16(%rdx), %rax
    pushq %rax
    ret

co_resume:
    movq (%rcx),%rdx
    movq 8(%rcx),%rcx
    jmp swap_ctx
#endif
