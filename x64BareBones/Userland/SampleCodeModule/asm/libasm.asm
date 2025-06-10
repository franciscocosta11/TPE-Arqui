GLOBAL syscall
GLOBAL triggerInvalidOpcode

section .text

syscall:
    push rbp
    mov rbp, rsp
    
    ; Mover el número de syscall de rdi a rax
    mov rax, rdi
    
    int 80h         ; Llamar a la interrupción de sistema
    
    mov rsp, rbp
    pop rbp
    ret

triggerInvalidOpcode:
    ud2             ; Ejecutar instrucción inválida
    ret             