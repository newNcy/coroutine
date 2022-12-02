; ms_context.asm
.code
public swap_ctx
swap_ctx proc 
    ;return address
    mov rax, QWORD PTR [rsp]
    mov QWORD PTR [rcx], rsp
    mov QWORD PTR [rcx+8], rbp
    mov QWORD PTR [rcx+16], rax
    mov QWORD PTR [rcx+24], rdi
    mov QWORD PTR [rcx+32], rsi
    mov QWORD PTR [rcx+56], rbx
    mov QWORD PTR [rcx+64], r12
    mov QWORD PTR [rcx+72], r13
    mov QWORD PTR [rcx+80], r14
    mov QWORD PTR [rcx+88], r15

    ;restore stack and ret address
    mov rsp, QWORD PTR [rdx]
    mov rbp, QWORD PTR [rdx + 8]
    mov rax, QWORD PTR [rdx + 16]
    mov QWORD PTR [rsp], rax

    ; callee save register restoring
    mov rdi, QWORD PTR [rdx + 24]
    mov rsi, QWORD PTR [rdx + 32]
    mov rbx, QWORD PTR [rdx + 56]
    mov r12, QWORD PTR [rdx + 64]
    mov r13, QWORD PTR [rdx + 72]
    mov r14, QWORD PTR [rdx + 80]
    mov r15, QWORD PTR [rdx + 88]
    ; put return value
    mov rax, QWORD PTR [rdx + 96]

    ; pass args when enter function
    mov rcx, QWORD PTR [rdx+40]
    mov rdx, QWORD PTR [rdx+48]

    ret
swap_ctx endp
end 

