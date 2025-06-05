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

void getRegisters(uint64_t *regs) {
    syscall(SYS_REGISTERS, (uint64_t)regs, 0, 0);
}

void triggerException(int exceptionNumber) {
    syscall(SYS_EXCEPTION, (uint64_t)exceptionNumber, 0, 0);
}

void setFontSize(uint8_t size) {
    syscall(SYS_FONT_SIZE, size, 0, 0);
}

int setTextColor(char* colorName) {
    const char* valid_colors[] = {
        "white", "red", "green", "blue", "yellow", "cyan", 
        "magenta", "orange", "purple", "pink", "gray", 
        "lightgray", "darkgray", 0
    };
    
    for (int i = 0; valid_colors[i] != 0; i++) {
        if (strcmp(colorName, valid_colors[i]) == 0) {
            syscall(6, (uint64_t)colorName, 0, 0); 
            return 1;
        }
    }
    
    return 0;
}

void printAvailableColors(void) {
    print("Colores disponibles: white, red, green, blue, yellow, cyan, magenta, orange, purple, pink, gray, lightgray, darkgray\n");
}

void fillScreen(uint32_t color) {
    syscall(8, (uint64_t)color, 0, 0);
}

char getKeyNonBlocking(void) {
    char key = 0;
    syscall(9, (uint64_t)&key, 0, 0);
    return key;
}

// Funciones matemáticas básicas
int abs(int x) {
    return x < 0 ? -x : x;
}

int isqrt(int x) {
    if (x == 0) return 0;
    
    int guess = x / 2;
    int prev_guess = 0;
    
    while (guess != prev_guess) {
        prev_guess = guess;
        guess = (guess + x / guess) / 2;
    }
    
    return guess;
}

int strlen(const char* str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

void putPixel(uint32_t color, uint64_t x, uint64_t y) {
    syscall(11, (uint64_t)color, x, y);
}

void drawRectangle(uint32_t color, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    for (int y = y1; y < y2; y++) {
        for (int x = x1; x < x2; x++) {
            putPixel(color, x, y);
        }
    }
}

// SOLO LAS 3 FUNCIONES DE SONIDO NECESARIAS
void playBeep(void) {
    syscall(13, 0, 0, 0);
}

void playWinSound(void) {
    syscall(14, 0, 0, 0);
}

void playSound(uint32_t frequency, uint32_t duration) {
    syscall(10, frequency, duration, 0);
}