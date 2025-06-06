#ifndef LIBC_H
#define LIBC_H

#include <stdint.h>
#include <stdarg.h>

// Números de syscall
#define SYS_READ  0
#define SYS_WRITE 1
#define SYS_CLEAR  2
#define SYS_TIME       3
#define SYS_REGISTERS  4
#define SYS_EXCEPTION  6
#define SYS_FONT_SIZE  5

#define SYS_PLAY_SOUND     10
#define SYS_PLAY_BEEP      13  
#define SYS_PLAY_WIN_SOUND 14
#define SYS_SOUND_OFF      15

/**
 * @brief Reproduce un sonido personalizado con frecuencia y duración específicas
 * @param frequency Frecuencia del sonido en Hz
 * @param duration Duración del sonido en milisegundos
 */
void playSound(uint32_t frequency, uint32_t duration);

/**
 * @brief Detiene cualquier sonido que se esté reproduciendo
 */
void soundOff(void);

/**
 * @brief Realiza una llamada al sistema con hasta 3 argumentos
 * @param syscall_number Número identificador de la syscall
 * @param arg1 Primer argumento de la syscall
 * @param arg2 Segundo argumento de la syscall
 * @param arg3 Tercer argumento de la syscall
 */
uint64_t syscall(uint64_t syscall_number, uint64_t arg1, uint64_t arg2, uint64_t arg3);

// Funciones C de alto nivel

/**
 * @brief Imprime un carácter en la pantalla
 * @param c Carácter a imprimir
 */
void putchar(char c);

/**
 * @brief Lee un carácter del teclado (bloqueante)
 */
char getchar(void);

/**
 * @brief Imprime una cadena de texto en la pantalla
 * @param s Cadena de texto a imprimir
 */
void print(const char *s);

/**
 * @brief Imprime una cadena de texto seguida de un salto de línea
 * @param s Cadena de texto a imprimir
 */
void println(const char *s);

/**
 * @brief Compara dos cadenas de texto
 * @param s1 Primera cadena a comparar
 * @param s2 Segunda cadena a comparar
 */
int strcmp(const char *s1, const char *s2);

/**
 * @brief Limpia la pantalla completamente
 */
void clearScreen(void);

/**
 * @brief Obtiene la hora actual del sistema
 * @param buffer Buffer donde se almacenará la hora en formato string
 */
void getTime(char *buffer);

/**
 * @brief Obtiene los valores actuales de los registros del procesador
 * @param regs Array donde se almacenarán los valores de los registros
 */
void getRegisters(uint64_t *regs);

/**
 * @brief Dispara una excepción específica para testing
 * @param exceptionNumber Número de la excepción a generar
 */
void triggerException(int exceptionNumber);

/**
 * @brief Cambia el tamaño de la fuente del texto
 * @param size Nuevo tamaño de fuente (1=pequeño, 2=mediano, 3=grande)
 */
void setFontSize(uint8_t size);

/**
 * @brief Establece el color del texto usando un nombre de color
 * @param colorName Nombre del color en string (ej: "red", "blue")
 */
int setTextColor(char* colorName);

/**
 * @brief Muestra en pantalla todos los colores disponibles
 */
void printAvailableColors(void);

/**
 * @brief Llena toda la pantalla con un color específico
 * @param color Color en formato hexadecimal (RGB)
 */
void fillScreen(uint32_t color);

/**
 * @brief Lee una tecla del teclado sin bloquear (no bloqueante)
 */
char getKeyNonBlocking(void);

/**
 * @brief Reproduce un sonido tipo beep corto
 */
void playBeep(void);

/**f
 * @brief Reproduce un sonido de victoria más elaborado
 */
void playWinSound(void);

/**
 * @brief Dibuja un píxel individual en la pantalla
 * @param color Color del píxel en formato hexadecimal
 * @param x Coordenada X del píxel
 * @param y Coordenada Y del píxel
 */
void putPixel(uint32_t color, uint64_t x, uint64_t y);

/**
 * @brief Dibuja un rectángulo lleno en la pantalla
 * @param color Color del rectángulo en formato hexadecimal
 * @param x1 Coordenada X de la esquina superior izquierda
 * @param y1 Coordenada Y de la esquina superior izquierda
 * @param x2 Coordenada X de la esquina inferior derecha
 * @param y2 Coordenada Y de la esquina inferior derecha
 */
void drawRectangle(uint32_t color, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

// Funciones matemáticas

/**
 * @brief Calcula el valor absoluto de un número entero
 * @param x Número del cual calcular el valor absoluto
 */
int abs(int x);

/**
 * @brief Calcula la raíz cuadrada entera de un número
 * @param x Número del cual calcular la raíz cuadrada
 */
int isqrt(int x);

/**
 * @brief Calcula la longitud de una cadena de texto
 * @param str Cadena de texto a medir
 */
int strlen(const char* str);

// Función del juego

/**
 * @brief Inicia el juego de mini golf
 */
void startGolfGame(void);

/**
 * @brief Imprime texto formateado (versión simplificada)
 * @param format String de formato con especificadores %d, %s, %c
 * @param ... Argumentos variables
 */
int printf(const char *format, ...);

/**
 * @brief Lee entrada formateada del usuario (versión simplificada)
 * @param format String de formato con especificadores %d, %s, %c
 * @param ... Punteros a variables donde almacenar los valores
 */
int scanf(const char *format, ...);

#endif