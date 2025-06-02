#ifndef VIDEO_DRIVER_H
#define VIDEO_DRIVER_H

#include <stdint.h>


#define SCREEN_WIDTH_PIXELS 1024
#define SCREEN_HEIGHT_PIXELS 768
#define WIDTH_S 8
#define HEIGHT_S 16
#define MAX_FONT_SIZE 3

//
void putPixel(uint32_t hexColor, uint64_t x, uint64_t y);
void vdPrintChar(char character);
void vdPrint(const char *string);
uint64_t vdPrintCharStyled(char character, uint32_t color, uint32_t bgColor);
void vdPrintStyled(char *s, uint32_t color, uint32_t bgColor);
uint64_t vdNPrintStyled(const char *string, uint32_t color, uint32_t bgColor, uint64_t N);
void vdNewline();
void vdClear();
void vdDelete();
void drawRectangle(uint32_t color, uint16_t up_l_x, uint16_t up_l_y, uint16_t lo_r_x, uint16_t lo_r_y);
void vdSetFontSize(uint8_t size);

void vdSetColor(uint32_t fgColor);
uint32_t vdGetColorByName(const char* colorName);
void vdPrintAvailableColors();

// Función para strcmp (si no está definida en otro lugar)
int strcmp(const char *s1, const char *s2);

#endif