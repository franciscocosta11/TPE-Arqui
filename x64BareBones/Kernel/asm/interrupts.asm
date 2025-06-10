

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
    
    ; Send EOI to the PIC
    mov al, 20h
    out 20h, al
    
    popState
    iretq

_irq01Handler:
    pushState
    
    mov rdi, 1  ; IRQ 1 para teclado
    call irqDispatcher
    
    ; Send EOI to the PIC
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
    
    ; Restaurar todos los registros EXCEPTO rax
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
    
    ; rax se preserva automáticamente con el valor de retorno
    iretq