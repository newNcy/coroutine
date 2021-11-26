.global  save_ctx, restore_ctx, 

save_ctx:
    movq %rsp, (%rdi);
    movq %rbp, 8(%rdi);
    movq (%rsp), %rdx;
    movq %rdx, 16(%rdi);
    movq $0, %rax;
    ret

restore_ctx:
    movq (%rdi), %rsp;
    movq 8(%rdi), %rbp;
    movq 16(%rdi), %rdx;
    movq %rdx, (%rsp);
    movq $1, %rax
    ret
    
