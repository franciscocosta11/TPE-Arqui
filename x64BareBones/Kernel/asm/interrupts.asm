; Kernel/asm/interrupts.asm
BITS 64

    EXTERN exception_handler_div0
    EXTERN exception_handler_invalid_opcode

    GLOBAL picMasterMask
    GLOBAL picSlaveMask
    GLOBAL _sti
    GLOBAL _exception0Handler
    GLOBAL _exception06Handler
    GLOBAL _irq00Handler
    GLOBAL _irq01Handler
    GLOBAL _syscallHandler

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
    mov al, dil
    out 0x21, al
    ret

picSlaveMask:
    mov al, dil
    out 0xA1, al
    ret

_sti:
    sti
    ret


_exception0Handler:
    cli
    pushState
    mov rdi, rsp
    call exception_handler_div0
    popState
    sti
    iretq

_exception06Handler:
    cli
    pushState
    mov rdi, rsp
    call exception_handler_invalid_opcode
    popState
    sti
    iretq


_irq00Handler:
    pushState
    popState
    iretq

_irq01Handler:
    pushState
    popState
    iretq


_syscallHandler:
    pushState
    popState
    iretq
