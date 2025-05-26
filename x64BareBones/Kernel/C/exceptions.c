#include <stdint.h>
#include <naiveConsole.h>

typedef struct {
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, r8,  r9;
    uint64_t r10, r11, r12, r13;
    uint64_t r14, r15;
} registers_t;

void exception_handler_div0(registers_t * regs);
void exception_handler_invalid_opcode(registers_t * regs);

static void print_registers(registers_t * r) {
    ncPrint(" RAX: 0x"); ncPrintHex(r->rax); ncNewline();
    ncPrint(" RBX: 0x"); ncPrintHex(r->rbx); ncNewline();
    ncPrint(" RCX: 0x"); ncPrintHex(r->rcx); ncNewline();
    ncPrint(" RDX: 0x"); ncPrintHex(r->rdx); ncNewline();
    ncPrint(" RSI: 0x"); ncPrintHex(r->rsi); ncNewline();
    ncPrint(" RDI: 0x"); ncPrintHex(r->rdi); ncNewline();
    ncPrint(" R8 : 0x"); ncPrintHex(r->r8 ); ncNewline();
    ncPrint(" R9 : 0x"); ncPrintHex(r->r9 ); ncNewline();
    ncPrint("R10 : 0x"); ncPrintHex(r->r10); ncNewline();
    ncPrint("R11 : 0x"); ncPrintHex(r->r11); ncNewline();
    ncPrint("R12 : 0x"); ncPrintHex(r->r12); ncNewline();
    ncPrint("R13 : 0x"); ncPrintHex(r->r13); ncNewline();
    ncPrint("R14 : 0x"); ncPrintHex(r->r14); ncNewline();
    ncPrint("R15 : 0x"); ncPrintHex(r->r15); ncNewline();
}

void exception_handler_div0(registers_t * regs) {
    ncPrint("[EXCEPTION] Division by zero\n");
    ncNewline();
    print_registers(regs);
    ncPrint(" Returning to shell...\n");
    ncNewline();
}

void exception_handler_invalid_opcode(registers_t * regs) {
    ncPrint("[EXCEPTION] Invalid opcode\n");
    ncNewline();
    print_registers(regs);
    ncPrint(" Returning to shell...\n");
    ncNewline();
}
