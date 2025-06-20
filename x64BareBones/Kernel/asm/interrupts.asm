;; ===================== interrupts.asm =====================
EXTERN keyboard_irq_handler
EXTERN irqDispatcher
EXTERN syscallDispatcher
GLOBAL _exception0Handler
GLOBAL _exception06Handler
GLOBAL picMasterMask
GLOBAL picSlaveMask
GLOBAL _sti
GLOBAL _irq00Handler
GLOBAL _irq01Handler
GLOBAL _syscallHandler
GLOBAL _hlt
GLOBAL _cli
GLOBAL capture_provisoria
GLOBAL capture_definitiva


section .bss
    capture_provisoria: resq 19
    capture_definitiva: resq 19

section .text

%macro pushState 0
    push rax
    push rbx
    push rcx
    push rdx
    push rbp
    push rdi
    push rsi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

%macro popState 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rsi
    pop rdi
    pop rbp
    pop rdx
    pop rcx
    pop rbx
    pop rax
%endmacro

picMasterMask:
    push rbp
    mov rbp, rsp
    mov al, dil
    out 0x21, al
    pop rbp
    ret

picSlaveMask:
    push rbp
    mov rbp, rsp
    mov al, dil
    out 0xA1, al
    pop rbp
    ret

_sti:
    sti
    ret

_hlt:
    sti
    hlt
    ret

_cli:
    cli
    ret

_irq00Handler:
    pushState

    mov rdi, 0  ; IRQ 0 para timer
    call irqDispatcher

    mov al, 20h
    out 20h, al

    popState
    iretq

_irq01Handler:
    ;; CAPTURA DE REGISTROS EN ORDEN CORRECTO
    mov [capture_provisoria + 8*0], rax
    mov [capture_provisoria + 8*1], rbx
    mov [capture_provisoria + 8*2], rcx
    mov [capture_provisoria + 8*3], rdx
    mov [capture_provisoria + 8*4], rsi
    mov [capture_provisoria + 8*5], rdi
    mov [capture_provisoria + 8*6], rsp
    mov [capture_provisoria + 8*7], rbp
    mov [capture_provisoria + 8*8], r8
    mov [capture_provisoria + 8*9], r9
    mov [capture_provisoria + 8*10], r10
    mov [capture_provisoria + 8*11], r11
    mov [capture_provisoria + 8*12], r12
    mov [capture_provisoria + 8*13], r13
    mov [capture_provisoria + 8*14], r14
    mov [capture_provisoria + 8*15], r15

    mov rax, [rsp]         ; RIP
    mov [capture_provisoria + 8*16], rax
    mov ax, cs
    movzx rax, ax
    mov [capture_provisoria + 8*17], rax
    pushfq
    pop rax
    mov [capture_provisoria + 8*18], rax

    pushState
    mov rdi, 1
    call irqDispatcher

    mov al, 20h
    out 20h, al

    popState
    iretq


_syscallHandler:
    ; Guardar todos los registros EXCEPTO rax
    push rbx
    push rcx
    push rdx
    push rbp
    push rdi
    push rsi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    call syscallDispatcher

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rsi
    pop rdi
    pop rbp
    pop rdx
    pop rcx
    pop rbx

    iretq
