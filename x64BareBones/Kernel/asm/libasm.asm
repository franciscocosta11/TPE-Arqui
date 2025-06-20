GLOBAL cpuVendor
GLOBAL inb
GLOBAL outb
GLOBAL saveRegisters

EXTERN capture_definitiva

section .text

inb:
    mov rdx, rdi
    in al, dx
    ret

outb:
    mov rdx, rdi
    mov rax, rsi
    out dx, al
    ret

cpuVendor:
    push rbp
    mov rbp, rsp
    push rbx

    mov rax, 0
    cpuid

    mov [rdi], ebx
    mov [rdi + 4], edx
    mov [rdi + 8], ecx
    mov byte [rdi+13], 0

    mov rax, rdi

    pop rbx
    mov rsp, rbp
    pop rbp
    ret

saveRegisters:
    mov rsi, capture_definitiva
    mov rcx, 0
.loop:
    cmp rcx, 19
    je .done
    mov rax, [rsi + rcx*8]
    mov [rdi + rcx*8], rax
    inc rcx
    jmp .loop
.done:
    ret
