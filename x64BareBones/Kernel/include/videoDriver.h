#ifndef VIDEO_DRIVER_H
#define VIDEO_DRIVER_H

#include <stdint.h>

// Definiciones de resolución y fuente
#define SCREEN_WIDTH_PIXELS 1024
#define SCREEN_HEIGHT_PIXELS 768
#define WIDTH_S 8
#define HEIGHT_S 16
#define MAX_FONT_SIZE 3

// Funciones básicas de video

/**
 * @brief Dibuja un píxel en la pantalla
 * @param hexColor Color del píxel en formato RGB hexadecimal (0xRRGGBB)
 * @param x Coordenada X del píxel (0 a SCREEN_WIDTH_PIXELS-1)
 * @param y Coordenada Y del píxel (0 a SCREEN_HEIGHT_PIXELS-1)
 */
void putPixel(uint32_t hexColor, uint64_t x, uint64_t y);

/**
 * @brief Imprime un carácter en la posición actual del cursor
 * @param character Carácter a imprimir
 * @note Usa el color y tamaño de fuente actuales
 */
void vdPrintChar(char character);

/**
 * @brief Imprime una cadena de caracteres
 * @param string Puntero a la cadena terminada en '\0'
 * @note Usa el color y tamaño de fuente actuales
 */
void vdPrint(const char *string);

/**
 * @brief Imprime un carácter con colores específicos
 * @param character Carácter a imprimir
 * @param color Color del texto en formato RGB hexadecimal
 * @param bgColor Color de fondo en formato RGB hexadecimal
 * @return Número de píxeles escritos
 */
uint64_t vdPrintCharStyled(char character, uint32_t color, uint32_t bgColor);

/**
 * @brief Imprime una cadena con colores específicos
 * @param s Puntero a la cadena terminada en '\0'
 * @param color Color del texto en formato RGB hexadecimal
 * @param bgColor Color de fondo en formato RGB hexadecimal
 */
void vdPrintStyled(char *s, uint32_t color, uint32_t bgColor);

/**
 * @brief Imprime N caracteres de una cadena con colores específicos
 * @param string Puntero a la cadena
 * @param color Color del texto en formato RGB hexadecimal
 * @param bgColor Color de fondo en formato RGB hexadecimal
 * @param N Número máximo de caracteres a imprimir
 * @return Número de caracteres efectivamente impresos
 */
uint64_t vdNPrintStyled(const char *string, uint32_t color, uint32_t bgColor, uint64_t N);

/**
 * @brief Mueve el cursor a la siguiente línea
 * @note Incrementa la posición Y del cursor y resetea X a 0
 */
void vdNewline(void);

/**
 * @brief Limpia toda la pantalla
 * @note Llena la pantalla con color negro y resetea el cursor a (0,0)
 */
void vdClear(void);

/**
 * @brief Borra el último carácter impreso (backspace)
 * @note Mueve el cursor hacia atrás y borra el carácter
 */
void vdDelete(void);

/**
 * @brief Dibuja un rectángulo relleno en la pantalla
 * @param color Color del rectángulo en formato RGB hexadecimal
 * @param up_l_x Coordenada X de la esquina superior izquierda
 * @param up_l_y Coordenada Y de la esquina superior izquierda
 * @param lo_r_x Coordenada X de la esquina inferior derecha
 * @param lo_r_y Coordenada Y de la esquina inferior derecha
 */
void drawRectangle(uint32_t color, uint16_t up_l_x, uint16_t up_l_y, uint16_t lo_r_x, uint16_t lo_r_y);

/**
 * @brief Establece el tamaño de la fuente
 * @param size Tamaño de fuente (1 a MAX_FONT_SIZE)
 * @note Tamaños mayores incrementan el tamaño de los caracteres
 */
void vdSetFontSize(uint8_t size);

/**
 * @brief Establece el color de primer plano por defecto
 * @param fgColor Color en formato RGB hexadecimal (0xRRGGBB)
 */
void vdSetColor(uint32_t fgColor);

/**
 * @brief Obtiene un color por su nombre
 * @param colorName Nombre del color (ej: "red", "blue", "green")
 * @return Valor RGB hexadecimal del color, o 0xFFFFFF si no se encuentra
 */
uint32_t vdGetColorByName(const char* colorName);

/**
 * @brief Imprime la lista de colores disponibles en el sistema
 * @note Muestra nombres y valores hexadecimales de colores predefinidos
 */
void vdPrintAvailableColors(void);

/**
 * @brief Llena toda la pantalla con un color específico
 * @param color Color de relleno en formato RGB hexadecimal
 */
void vdFillScreen(uint32_t color);

/**
 * @brief Compara dos cadenas de caracteres
 * @param s1 Primera cadena a comparar
 * @param s2 Segunda cadena a comparar
 * @return 0 si son iguales, valor negativo si s1 < s2, positivo si s1 > s2
 * @note Función auxiliar para comparación de nombres de colores
 */
int strcmp(const char *s1, const char *s2);

/**
 * @brief Obtiene el ancho actual de la pantalla
 * @return Ancho en píxeles
 */
uint16_t vdGetScreenWidth(void);

/**
 * @brief Obtiene la altura actual de la pantalla
 * @return Altura en píxeles  
 */
uint16_t vdGetScreenHeight(void);

#endif