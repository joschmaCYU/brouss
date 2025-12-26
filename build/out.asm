section .data
    str0_len dq 6
    str0 db "loop\n"
    str1_len dq 4
    str1 db "rrhh"
    str2_len dq 7
    str2 db "hello\n"
global _start
section .text
_start:
    mov rax, 3
    push rax
    ;; while
label0:
    push QWORD [rsp + 0]
    pop rax
    test rax, rax
    jz label1
    mov rax, 6
    push rax
    lea rax, [rel str0]
    push rax
    pop rsi
    pop rdx
    mov rax, 1
    mov rdi, 1
    syscall
    mov rax, 1
    push rax
    push QWORD [rsp + 8]
    pop rax
    pop rbx
    sub rax, rbx
    push rax
    pop rax
    mov QWORD [rsp + 0], rax
    add rsp, 0
    jmp label0
label1:
    ;; /while
    mov rax, 4
    push rax
    lea rax, [rel str1]
    push rax
    mov rax, 7
    push rax
    lea rax, [rel str2]
    push rax
    pop rax
    pop rbx
    mov QWORD [rsp + 8], rbx
    mov QWORD [rsp + 0], rax
    push QWORD [rsp + 8]
    push QWORD [rsp + 8]
    pop rsi
    pop rdx
    mov rax, 1
    mov rdi, 1
    syscall
    mov rax, 0
    push rax
    mov rax, 60
    pop rdi
    syscall
    mov rax, 60
    mov rdi, 0
    syscall
