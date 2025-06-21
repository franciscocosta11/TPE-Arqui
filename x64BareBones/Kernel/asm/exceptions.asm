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
    pushState
    
    mov rdi, 0          ; ID de excepción
    mov rsi, rsp        ; Puntero a registros
    
    call exceptionDispatcher
    
    popState
    ; NO retornar a la instrucción problemática
    ; En lugar de eso, ajustar RIP para saltar la instrucción
    mov rax, [rsp]      ; Obtener RIP del stack
    add rax, 2          ; Saltar la instrucción div (2 bytes aprox)
    mov [rsp], rax      ; Actualizar RIP en el stack
    iretq

_exception06Handler:
    pushState
    
    mov rdi, 6          ; ID de excepción  
    mov rsi, rsp        ; Puntero a registros
    
    call exceptionDispatcher
    
    popState
    ; Para invalid opcode, también saltar la instrucción problemática
    mov rax, [rsp]      ; Obtener RIP del stack
    add rax, 2          ; Saltar la instrucción inválida
    mov [rsp], rax      ; Actualizar RIP en el stack
    iretq