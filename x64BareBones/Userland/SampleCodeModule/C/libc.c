#include "./../include/libc.h"

void putchar(char c) {
    char buf[2] = { c, 0 };
    syscall(SYS_WRITE, (uint64_t)buf, 0, 0);
}

char getchar(void) {
    char c;
    syscall(SYS_READ, (uint64_t)&c, 0, 0);
    return c;
}

void print(const char *s) {
    syscall(SYS_WRITE, (uint64_t)s, 0, 0);
}

void println(const char *s) {
    print(s);
    putchar('\n');
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

void clearScreen(void) {
    syscall(SYS_CLEAR, 0, 0, 0);
}

void getTime(char *buffer) {
    syscall(SYS_TIME, (uint64_t)buffer, 0, 0);
}

// Obtiene los valores de los registros
void getRegisters(uint64_t *regs) {
    syscall(SYS_REGISTERS, (uint64_t)regs, 0, 0);
}

// Genera una excepción específica
void triggerException(int exceptionNumber) {
    syscall(SYS_EXCEPTION, (uint64_t)exceptionNumber, 0, 0);
}

void setFontSize(uint8_t size) {
    syscall(SYS_FONT_SIZE, size, 0, 0);
}