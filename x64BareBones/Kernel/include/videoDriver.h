#include <stdint.h>
#include <font.h>
#ifndef VIDEO_DRIVER_H
#define VIDEO_DRIVER_H

/*************************************
**
** DECLARACIONES DE CONSTANTES
**
**************************************/

#define SCREEN_WIDTH_PIXELS 1024
#define SCREEN_HEIGHT_PIXELS 768
#define MARGIN_SIZE 1

#define SMALL_FONT 1
#define LARGE_FONT 2

#define BITS_PER_PIXEL 3

/*************************************
**
** DECLARACIONES DE FUNCIONES
**
**************************************/

unsigned char *getCharHexData(uint8_t c);
uint8_t *getLargeCharHexData(uint8_t c);
void vdPrint(const char *string);
void vdPrintChar(char character);
uint64_t vdPrintCharStyled(char character, uint32_t color, uint32_t bgColor);
void vdPrintStyled(char *s, uint32_t color, uint32_t bgColor);
uint64_t vdNPrintStyled(const char *string, uint32_t color, uint32_t bgColor, uint64_t N);
void vdNewline();
void vdPrintDec(uint64_t value);
void vdPrintHex(uint64_t value);
void vdPrintBin(uint64_t value);
void vdPrintBase(uint64_t value, uint32_t base);
void vdClear();
void vdDelete();
void vdChangeFontSize();
void drawRectangle(uint32_t color, uint16_t up_l_x, uint16_t up_l_y, uint16_t lo_r_x, uint16_t lo_r_y);

/*************************************
**
** DECLARACIONES DE STRUCTS
**
**************************************/

typedef struct
{
    uint16_t up_l_x;
    uint16_t up_l_y;
    uint16_t lo_r_y;
    uint16_t lo_r_x;
    uint32_t color;
} Rectangle;

#endif