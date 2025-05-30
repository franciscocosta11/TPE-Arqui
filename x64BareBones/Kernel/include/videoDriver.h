#include <stdint.h>
#ifndef VIDEO_DRIVER_H
#define VIDEO_DRIVER_H

#define WIDTH_S 8
#define HEIGHT_S 16

#define SCREEN_WIDTH_PIXELS 1024
#define SCREEN_HEIGHT_PIXELS 768
#define MARGIN_SIZE 1

#define BITS_PER_PIXEL 3

extern uint8_t currentFontSize;  
#define MAX_FONT_SIZE 3

unsigned char *getCharHexData(uint8_t c);
void vdPrint(const char *string);
void vdPrintChar(char character);
uint64_t vdPrintCharStyled(char character, uint32_t color, uint32_t bgColor);
void vdPrintStyled(char *s, uint32_t color, uint32_t bgColor);
uint64_t vdNPrintStyled(const char *string, uint32_t color, uint32_t bgColor, uint64_t N);
void vdNewline();
void vdClear();
void vdDelete();
void vdSetFontSize(uint8_t size);

void drawRectangle(uint32_t color, uint16_t up_l_x, uint16_t up_l_y, uint16_t lo_r_x, uint16_t lo_r_y);

typedef struct
{
    uint16_t up_l_x;
    uint16_t up_l_y;
    uint16_t lo_r_y;
    uint16_t lo_r_x;
    uint32_t color;
} Rectangle;

#endif