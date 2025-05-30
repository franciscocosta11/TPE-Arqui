EXTERN exceptionDispatcher
global _exception0Handler
global _exception06Handler

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

_exception0Handler:
    cli
    pushState
    
    ; Primer parámetro: ID de la excepción (0 para división por cero)
    mov rdi, 0
    ; Segundo parámetro: puntero a la estructura de registros
    mov rsi, rsp
    
    call exceptionDispatcher
    
    popState
    sti
    iretq

_exception06Handler:
    cli
    pushState
    
    ; Primer parámetro: ID de la excepción (6 para opcode inválido)
    mov rdi, 6
    ; Segundo parámetro: puntero a la estructura de registros
    mov rsi, rsp
    
    call exceptionDispatcher
    
    popState
    sti
    iretq