#ifndef LIBC_H
#define LIBC_H

#include <stdint.h>

// Números de syscall
#define SYS_READ  0
#define SYS_WRITE 1
#define SYS_CLEAR  2
#define SYS_TIME       3
#define SYS_REGISTERS  4
#define SYS_EXCEPTION  5
#define SYS_FONT_SIZE  6

// Función genérica para syscalls
uint64_t syscall(uint64_t syscall_number, uint64_t arg1, uint64_t arg2, uint64_t arg3);

// Funciones C de alto nivel
void putchar(char c);
char getchar(void);
void print(const char *s);
void println(const char *s);
int strcmp(const char *s1, const char *s2);
void clearScreen(void);
void getTime(char *buffer);
void getRegisters(uint64_t *regs);
void triggerException(int exceptionNumber);
void setFontSize(uint8_t size);
int setTextColor(char* colorName);
void printAvailableColors(void);

#endif