#include <stdint.h>
#include <videoDriver.h>
#include <keyboard.h>  

// Definición de IDs de excepciones
#define ZERO_EXCEPTION_ID 0
#define INVALID_OPCODE_ID 6

typedef struct {
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rsp, rbp;  
    uint64_t r8, r9, r10, r11;
    uint64_t r12, r13, r14, r15;
    uint64_t rip, rflags;         
} registers_t;

// Declaración de handlers específicos
static void zero_division(registers_t *regs);
static void invalid_opcode(registers_t *regs);

void exceptionDispatcher(uint64_t exception, uint64_t *stack_frame) {
    registers_t regs;
    
    // El stack_frame contiene registros en este orden (debido al pushState):
    // [0] = rax, [1] = rbx, [2] = rcx, [3] = rdx, [4] = rbp
    // [5] = rdi, [6] = rsi, [7] = r8, [8] = r9, [9] = r10
    // [10] = r11, [11] = r12, [12] = r13, [13] = r14, [14] = r15
    // [15] = RIP (de la excepción), [16] = CS, [17] = RFLAGS, [18] = RSP original
    
    regs.rax = stack_frame[0];
    regs.rbx = stack_frame[1]; 
    regs.rcx = stack_frame[2];
    regs.rdx = stack_frame[3];
    regs.rbp = stack_frame[4];
    regs.rdi = stack_frame[5];
    regs.rsi = stack_frame[6];
    regs.r8 = stack_frame[7];
    regs.r9 = stack_frame[8];
    regs.r10 = stack_frame[9];
    regs.r11 = stack_frame[10];
    regs.r12 = stack_frame[11];
    regs.r13 = stack_frame[12];
    regs.r14 = stack_frame[13];
    regs.r15 = stack_frame[14];
    regs.rip = stack_frame[15];     // RIP de la excepción (debería ser ~0x400000)
    regs.rflags = stack_frame[17];  // RFLAGS
    regs.rsp = stack_frame[18];     // RSP original

    switch (exception) {
        case ZERO_EXCEPTION_ID:
            zero_division(&regs);
            break;
        case INVALID_OPCODE_ID:
            invalid_opcode(&regs);
            break;
    }
    
    keyboard_clear_buffer();
}

// Función auxiliar para imprimir registros
static void print_hex(uint64_t value) {
    char buffer[17]; // 16 dígitos hex + null terminator
    
    for (int i = 0; i < 16; i++) {
        buffer[i] = "0123456789ABCDEF"[(value >> (60 - i*4)) & 0xF];
    }
    buffer[16] = '\0';
    
    vdPrint(buffer);
}

static void print_registers(registers_t *r) {
    const char *reg_names[] = {
        " RAX: 0x", " RBX: 0x", " RCX: 0x", " RDX: 0x",
        " RSI: 0x", " RDI: 0x", " RSP: 0x", " RBP: 0x",  
        " R8 : 0x", " R9 : 0x", " R10: 0x", " R11: 0x",
        " R12: 0x", " R13: 0x", " R14: 0x", " R15: 0x",
        " RIP: 0x", "RFLAGS: 0x"                          
    };
    
    uint64_t *reg_values[] = {
        &r->rax, &r->rbx, &r->rcx, &r->rdx,
        &r->rsi, &r->rdi, &r->rsp, &r->rbp,              
        &r->r8, &r->r9, &r->r10, &r->r11,
        &r->r12, &r->r13, &r->r14, &r->r15,
        &r->rip, &r->rflags                              
    };
    
    for (int i = 0; i < 18; i++) {                       
        vdPrint(reg_names[i]);
        print_hex(*reg_values[i]);
        vdPrint("\n");
    }
}

// Handler para división por cero
static void zero_division(registers_t *regs) {
    // Guardar color actual y cambiar a rojo
    vdSetColor(0xFF0000); // Rojo
    
    vdPrint("[EXCEPTION] Division by zero\n\n");
    print_registers(regs);
    vdPrint("\n Returning to shell...\n\n");
    
    // Restaurar color por defecto (blanco)
    vdSetColor(0xFFFFFF);
}

static void invalid_opcode(registers_t *regs) {
    // Guardar color actual y cambiar a rojo
    vdSetColor(0xFF0000); // Rojo
    
    vdPrint("[EXCEPTION] Invalid opcode\n\n");
    print_registers(regs);
    vdPrint("\n Returning to shell...\n\n");
    
    // Restaurar color por defecto (blanco)
    vdSetColor(0xFFFFFF);
}