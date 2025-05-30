#include <videoDriver.h>
#include <stdint.h>
#include <font.h>  // Incluir el nuevo archivo de fuente

uint8_t currentFontSize = 1;

struct vbe_mode_info_structure {
    uint16_t attributes;		// deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
    uint8_t window_a;			// deprecated
    uint8_t window_b;			// deprecated
    uint16_t granularity;		// deprecated; used while calculating bank numbers
    uint16_t window_size;
    uint16_t segment_a;
    uint16_t segment_b;
    uint32_t win_func_ptr;		// deprecated; used to switch banks from protected mode without returning to real mode
    uint16_t pitch;			// number of bytes per horizontal line
    uint16_t width;			// width in pixels
    uint16_t height;			// height in pixels
    uint8_t w_char;			// unused...
    uint8_t y_char;			// ...
    uint8_t planes;
    uint8_t bpp;			// bits per pixel in this mode
    uint8_t banks;			// deprecated; total number of banks in this mode
    uint8_t memory_model;
    uint8_t bank_size;		// deprecated; size of a bank, almost always 64 KB but may be 16 KB...
    uint8_t image_pages;
    uint8_t reserved0;
 
    uint8_t red_mask;
    uint8_t red_position;
    uint8_t green_mask;
    uint8_t green_position;
    uint8_t blue_mask;
    uint8_t blue_position;
    uint8_t reserved_mask;
    uint8_t reserved_position;
    uint8_t direct_color_attributes;
 
    uint32_t framebuffer;		// physical address of the linear frame buffer; write here to draw to the screen
    uint32_t off_screen_mem_off;
    uint16_t off_screen_mem_size;	// size of memory in the framebuffer but not being displayed on the screen
    uint8_t reserved1[206];
} __attribute__ ((packed));

typedef struct vbe_mode_info_structure * VBEInfoPtr;

VBEInfoPtr VBE_mode_info = (VBEInfoPtr) 0x0000000000005C00;

// Variables globales para manejar el cursor y el estilo de fuente
static uint16_t cursorX = 0;
static uint16_t cursorY = 0;
static uint32_t defaultFgColor = 0xFFFFFF; // Blanco
static uint32_t defaultBgColor = 0x000000; // Negro

void putPixel(uint32_t hexColor, uint64_t x, uint64_t y) {
    uint8_t * framebuffer = (uint8_t *)(uint64_t)VBE_mode_info->framebuffer;
    uint64_t offset = (x * ((VBE_mode_info->bpp)/8)) + (y * VBE_mode_info->pitch);
    framebuffer[offset]     =  (hexColor) & 0xFF;
    framebuffer[offset+1]   =  (hexColor >> 8) & 0xFF; 
    framebuffer[offset+2]   =  (hexColor >> 16) & 0xFF;
}

unsigned char *getCharHexData(uint8_t c) {
    return font8x16[c];
}

// Imprime un carácter con los colores por defecto
void vdPrintChar(char character) {
    vdPrintCharStyled(character, defaultFgColor, defaultBgColor);
}

// Imprime una cadena con los colores por defecto
void vdPrint(const char *string) {
    vdPrintStyled((char*)string, defaultFgColor, defaultBgColor);
}

uint64_t vdPrintCharStyled(char character, uint32_t color, uint32_t bgColor) {
    uint8_t fontWidth = WIDTH_S;
    uint8_t fontHeight = HEIGHT_S;
    
    unsigned char* charData = getCharHexData(character);
    
    if (character == '\n') {
        vdNewline();
        return 0;
    }
    
    if (character == '\b') {
        vdDelete();
        return 0;
    }
    
    if (cursorX + (fontWidth * currentFontSize) >= SCREEN_WIDTH_PIXELS) {
        vdNewline();
    }
    
    for (uint8_t y = 0; y < fontHeight; y++) {
        for (uint8_t x = 0; x < fontWidth; x++) {
            uint8_t row = charData[y];
            uint8_t isSet = row & (1 << (fontWidth - 1 - x));
            uint32_t pixelColor = isSet ? color : bgColor;
            
            // Dibujar múltiples píxeles para escalar el carácter
            for (uint8_t sy = 0; sy < currentFontSize; sy++) {
                for (uint8_t sx = 0; sx < currentFontSize; sx++) {
                    putPixel(pixelColor, 
                             cursorX + (x * currentFontSize) + sx, 
                             cursorY + (y * currentFontSize) + sy);
                }
            }
        }
    }
    
    cursorX += fontWidth * currentFontSize;
    
    return fontWidth * currentFontSize;
}

void vdPrintStyled(char *s, uint32_t color, uint32_t bgColor) {
    while (*s != 0) {
        vdPrintCharStyled(*s, color, bgColor);
        s++;
    }
}

uint64_t vdNPrintStyled(const char *string, uint32_t color, uint32_t bgColor, uint64_t N) {
    uint64_t count = 0;
   
    for (uint64_t i = 0; i < N && string[i] != 0; i++) {
        count += vdPrintCharStyled(string[i], color, bgColor);
    }
   
    return count;
}

// Realiza un salto de línea
void vdNewline() {
    cursorX = 0;
    cursorY += HEIGHT_S * currentFontSize;
    
    if (cursorY + (HEIGHT_S * currentFontSize) >= SCREEN_HEIGHT_PIXELS) {
        vdClear(); 
    }
}

// Limpia la pantalla y reinicia el cursor
void vdClear() {
    drawRectangle(defaultBgColor, 0, 0, SCREEN_WIDTH_PIXELS, SCREEN_HEIGHT_PIXELS);
    cursorX = 0;
    cursorY = 0;
}

void vdDelete() {
    uint8_t fontWidth = WIDTH_S;
    uint8_t fontHeight = HEIGHT_S;
    
    if (cursorX >= fontWidth * currentFontSize) {
        cursorX -= fontWidth * currentFontSize;
        
        for (uint8_t y = 0; y < fontHeight * currentFontSize; y++) {
            for (uint8_t x = 0; x < fontWidth * currentFontSize; x++) {
                putPixel(defaultBgColor, cursorX + x, cursorY + y);
            }
        }
    }
}

// Dibuja un rectángulo con un color específico
void drawRectangle(uint32_t color, uint16_t up_l_x, uint16_t up_l_y, uint16_t lo_r_x, uint16_t lo_r_y) {
    for (uint16_t y = up_l_y; y < lo_r_y; y++) {
        for (uint16_t x = up_l_x; x < lo_r_x; x++) {
            putPixel(color, x, y);
        }
    }
}

void vdSetFontSize(uint8_t size) {
    if (size >= 1 && size <= MAX_FONT_SIZE) {
        currentFontSize = size;
    }
}