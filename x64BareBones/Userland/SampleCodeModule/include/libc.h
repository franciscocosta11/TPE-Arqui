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

#define SYS_PLAY_SOUND     10
#define SYS_PLAY_BEEP      13  
#define SYS_PLAY_WIN_SOUND 14
#define SYS_SOUND_OFF      15

// Y agregar estas declaraciones de funciones:
void playSound(uint32_t frequency, uint32_t duration);
void soundOff(void);

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

void fillScreen(uint32_t color);
char getKeyNonBlocking(void);
void playBeep(void);
void playWinSound(void);
void putPixel(uint32_t color, uint64_t x, uint64_t y);
void drawRectangle(uint32_t color, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

// Funciones matemáticas
int abs(int x);
int isqrt(int x);  // Raíz cuadrada entera
int strlen(const char* str);

// Función del juego
void startGolfGame(void);



// Y las declaraciones:
void playSound(uint32_t frequency, uint32_t duration);
void playBeep(void);
void playWinSound(void);
void soundOff(void);

#endif