GLOBAL cpuVendor
GLOBAL inb
GLOBAL outb
GLOBAL saveRegisters

section .text
	
inb:
    mov rdx, rdi
	in al, dx
	ret

outb:
    mov rdx,rdi
    mov rax,rsi
    out dx,al
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
    mov [rdi], rax
    mov [rdi+8], rbx
    mov [rdi+16], rcx
    mov [rdi+24], rdx
    mov [rdi+32], rsi
    add rdi, 40
    mov [rdi], rdi
    sub rdi, 40
    mov [rdi+48], rsp
    mov [rdi+56], rbp
    mov [rdi+64], r8
    mov [rdi+72], r9
    mov [rdi+80], r10
    mov [rdi+88], r11
    mov [rdi+96], r12
    mov [rdi+104], r13
    mov [rdi+112], r14
    mov [rdi+120], r15
    
    ; Guardar rip (usando dirección de retorno en la pila)
    mov rax, [rsp]
    mov [rdi+128], rax
    
    ; Guardar cs, rflags
    mov rax, cs
    mov [rdi+136], rax
    
    pushfq
    pop rax
    mov [rdi+144], rax
    
    ret