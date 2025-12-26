section .data
    str0_len dq 4
    str0 db "rrhh"
global _start
section .text
_start:
    mov rax, 4
    push rax
    lea rax, [rel str0]
    push rax
    push QWORD [rsp + 8]
    push QWORD [rsp + 8]
    pop rsi
    pop rdx
    mov rax, 1
    mov rdi, 1
    syscall
    mov rax, 5
    push rax
    mov rax, 60
    pop rdi
    syscall
    mov rax, 60
    mov rdi, 0
    syscall
