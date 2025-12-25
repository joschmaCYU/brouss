global _start
_start:
    mov rax, 10
    push rax
    ;; if
    mov rax, 0
    push rax
    pop rax
    test rax, rax
    jz label0
    mov rax, 1
    push rax
    mov rax, 60
    pop rdi
    syscall
    add rsp, 0
    jmp label1
label0:
    ;; elif
    mov rax, 1
    push rax
    pop rax
    test rax, rax
    jz label2
    mov rax, 3
    push rax
    mov rax, 60
    pop rdi
    syscall
    add rsp, 0
    jmp label1
label2:
    ;; else
    mov rax, 4
    push rax
    mov rax, 60
    pop rdi
    syscall
    add rsp, 0
label1:
    ;; /if
    mov rax, 5
    push rax
    mov rax, 60
    pop rdi
    syscall
    mov rax, 60
    mov rdi, 0
    syscall
