#include <stdint.h>
#include <videoDriver.h>

// Definición de IDs de excepciones
#define ZERO_EXCEPTION_ID 0
#define INVALID_OPCODE_ID 6

typedef struct {
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, r8, r9;
    uint64_t r10, r11, r12, r13;
    uint64_t r14, r15;
} registers_t;

// Declaración de handlers específicos
static void zero_division(registers_t *regs);
static void invalid_opcode(registers_t *regs);

// Función principal que distribuye las excepciones
void exceptionDispatcher(int exception, registers_t *regs) {
    switch (exception) {
        case ZERO_EXCEPTION_ID:
            zero_division(regs);
            break;
        case INVALID_OPCODE_ID:
            invalid_opcode(regs);
            break;
    }
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

// Función simplificada para imprimir registros
static void print_registers(registers_t *r) {
    // Lista de nombres de registros y sus valores correspondientes
    const char *reg_names[] = {
        " RAX: 0x", " RBX: 0x", " RCX: 0x", " RDX: 0x",
        " RSI: 0x", " RDI: 0x", " R8 : 0x", " R9 : 0x",
        " R10: 0x", " R11: 0x", " R12: 0x", " R13: 0x",
        " R14: 0x", " R15: 0x"
    };
    
    uint64_t *reg_values[] = {
        &r->rax, &r->rbx, &r->rcx, &r->rdx,
        &r->rsi, &r->rdi, &r->r8, &r->r9,
        &r->r10, &r->r11, &r->r12, &r->r13,
        &r->r14, &r->r15
    };
    
    // Imprimir cada registro
    for (int i = 0; i < 14; i++) {
        vdPrint(reg_names[i]);
        print_hex(*reg_values[i]);
        vdPrint("\n");
    }
}

// Handler para división por cero
static void zero_division(registers_t *regs) {
    vdPrint("[EXCEPTION] Division by zero\n\n");
    print_registers(regs);
    vdPrint("\n Returning to shell...\n\n");
}

// Handler para código de operación inválido
static void invalid_opcode(registers_t *regs) {
    vdPrint("[EXCEPTION] Invalid opcode\n\n");
    print_registers(regs);
    vdPrint("\n Returning to shell...\n\n");
}