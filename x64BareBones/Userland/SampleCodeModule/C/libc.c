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

static void itoa(int num, char *str) {
    int i = 0;
    int isNegative = 0;
    
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    
    if (num < 0) {
        isNegative = 1;
        num = -num;
    }
    
    while (num != 0) {
        str[i++] = (num % 10) + '0';
        num = num / 10;
    }
    
    if (isNegative) {
        str[i++] = '-';
    }
    
    str[i] = '\0';
    
    // Invertir string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

// Función auxiliar para convertir string a entero
static int atoi(const char *str) {
    int result = 0;
    int sign = 1;
    int i = 0;
    
    // Manejar signo negativo
    if (str[0] == '-') {
        sign = -1;
        i = 1;
    }
    
    while (str[i] >= '0' && str[i] <= '9') {
        result = result * 10 + (str[i] - '0');
        i++;
    }
    
    return result * sign;
}

int printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    int chars_printed = 0;
    char buffer[32];
    
    for (int i = 0; format[i] != '\0'; i++) {
        if (format[i] == '%' && format[i + 1] != '\0') {
            i++; // Saltar el '%'
            
            switch (format[i]) {
                case 'd': {
                    int value = va_arg(args, int);
                    itoa(value, buffer);
                    print(buffer);
                    chars_printed += strlen(buffer);
                    break;
                }
                case 's': {
                    char *str = va_arg(args, char*);
                    if (str) {
                        print(str);
                        chars_printed += strlen(str);
                    }
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    putchar(c);
                    chars_printed++;
                    break;
                }
                case '%':
                    putchar('%');
                    chars_printed++;
                    break;
                default:
                    putchar('%');
                    putchar(format[i]);
                    chars_printed += 2;
                    break;
            }
        } else {
            putchar(format[i]);
            chars_printed++;
        }
    }
    
    va_end(args);
    return chars_printed;
}

int scanf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    char input_buffer[256];
    int buffer_index = 0;
    int items_read = 0;
    
    // Leer línea completa de entrada
    char c;
    while ((c = getchar()) != '\n' && c != '\0' && buffer_index < 255) {
        if (c == '\b') { // Backspace
            if (buffer_index > 0) {
                buffer_index--;
                print("\b \b"); // Borrar carácter visualmente
            }
        } else {
            input_buffer[buffer_index++] = c;
            putchar(c); // Echo del carácter
        }
    }
    input_buffer[buffer_index] = '\0';
    putchar('\n'); // Nueva línea después de Enter
    
    // Procesar formato
    int input_pos = 0;
    for (int i = 0; format[i] != '\0'; i++) {
        if (format[i] == '%' && format[i + 1] != '\0') {
            i++; // Saltar el '%'
            
            // Saltar espacios en blanco en input
            while (input_pos < buffer_index && 
                   (input_buffer[input_pos] == ' ' || input_buffer[input_pos] == '\t')) {
                input_pos++;
            }
            
            switch (format[i]) {
                case 'd': {
                    int *ptr = va_arg(args, int*);
                    char num_str[32];
                    int num_index = 0;
                    
                    // Leer dígitos (y signo negativo)
                    if (input_buffer[input_pos] == '-') {
                        num_str[num_index++] = input_buffer[input_pos++];
                    }
                    
                    while (input_pos < buffer_index && 
                           input_buffer[input_pos] >= '0' && 
                           input_buffer[input_pos] <= '9') {
                        num_str[num_index++] = input_buffer[input_pos++];
                    }
                    
                    if (num_index > 0) {
                        num_str[num_index] = '\0';
                        *ptr = atoi(num_str);
                        items_read++;
                    }
                    break;
                }
                case 's': {
                    char *str = va_arg(args, char*);
                    int str_index = 0;
                    
                    // Leer hasta espacio o final
                    while (input_pos < buffer_index && 
                           input_buffer[input_pos] != ' ' && 
                           input_buffer[input_pos] != '\t') {
                        str[str_index++] = input_buffer[input_pos++];
                    }
                    
                    if (str_index > 0) {
                        str[str_index] = '\0';
                        items_read++;
                    }
                    break;
                }
                case 'c': {
                    char *ptr = va_arg(args, char*);
                    if (input_pos < buffer_index) {
                        *ptr = input_buffer[input_pos++];
                        items_read++;
                    }
                    break;
                }
            }
        }
    }
    
    va_end(args);
    return items_read;
}

void soundOff(void) {
    syscall(15, 0, 0, 0);
}

uint16_t getScreenWidth(void) {
    return (uint16_t)syscall(SYS_GET_SCREEN_WIDTH, 0, 0, 0);
}

uint16_t getScreenHeight(void) {
    return (uint16_t)syscall(SYS_GET_SCREEN_HEIGHT, 0, 0, 0);
}