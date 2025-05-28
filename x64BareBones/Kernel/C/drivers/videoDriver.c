#include <videoDriver.h>
#include <font.h>
#include <stdint.h>

// Fuente pequeña (8x16)
unsigned char font[256][16] = {
    [' '] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    ['A'] = {0x00,0x18,0x3C,0x3C,0x66,0x66,0x7E,0x7E,0x66,0x66,0x66,0x66,0x00,0x00,0x00,0x00},
    ['B'] = {0x00,0x7C,0x66,0x66,0x7C,0x66,0x66,0x66,0x66,0x66,0x7C,0x00,0x00,0x00,0x00,0x00},
    ['C'] = {0x00,0x3C,0x66,0x60,0x60,0x60,0x60,0x60,0x60,0x66,0x3C,0x00,0x00,0x00,0x00,0x00},
    ['0'] = {0x00,0x3C,0x66,0x6E,0x7E,0x76,0x66,0x66,0x66,0x66,0x3C,0x00,0x00,0x00,0x00,0x00},
    ['1'] = {0x00,0x18,0x38,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x7E,0x00,0x00,0x00,0x00,0x00},
    // … completar el resto de caracteres …
};

// Fuente grande (16x32)
unsigned char largefont[256][32] = {
    // Aquí iría la definición completa de la fuente grande (16x32)
    // Esto es solo un ejemplo
    [' '] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // ... y así sucesivamente para todos los caracteres
};

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
static uint8_t fontStyle = SMALL_FONT; // 1 para fuente pequeña, 2 para fuente grande
static uint32_t defaultFgColor = 0xFFFFFF; // Blanco
static uint32_t defaultBgColor = 0x000000; // Negro

// Fuentes (se definirán en otro archivo)
extern unsigned char font[][16];
extern unsigned char largefont[][32];

void putPixel(uint32_t hexColor, uint64_t x, uint64_t y) {
    uint8_t * framebuffer = (uint8_t *) VBE_mode_info->framebuffer;
    uint64_t offset = (x * ((VBE_mode_info->bpp)/8)) + (y * VBE_mode_info->pitch);
    framebuffer[offset]     =  (hexColor) & 0xFF;
    framebuffer[offset+1]   =  (hexColor >> 8) & 0xFF; 
    framebuffer[offset+2]   =  (hexColor >> 16) & 0xFF;
}

// Obtiene los datos hexadecimales de un carácter (fuente pequeña)
unsigned char *getCharHexData(uint8_t c) {
    return font[c];
}

// Obtiene los datos hexadecimales de un carácter (fuente grande)
uint8_t *getLargeCharHexData(uint8_t c) {
    return largefont[c];
}

// Dibuja un carácter con estilo en la posición actual del cursor
uint64_t vdPrintCharStyled(char character, uint32_t color, uint32_t bgColor) {
    uint8_t fontWidth = (fontStyle == SMALL_FONT) ? WIDTH_S : WIDTH_L;
    uint8_t fontHeight = (fontStyle == SMALL_FONT) ? HEIGHT_S : HEIGHT_L;
    
    unsigned char* charData = (fontStyle == SMALL_FONT) ? 
        getCharHexData(character) : getLargeCharHexData(character);
        
    if (character == '\n') {
        vdNewline();
        return 0;
    }
    
    if (character == '\b') {
        vdDelete();
        return 0;
    }
    
    // Verificar si necesitamos hacer un salto de línea
    if (cursorX + fontWidth >= SCREEN_WIDTH_PIXELS) {
        vdNewline();
    }
    
    // Dibujar el carácter pixel por pixel
    for (uint8_t y = 0; y < fontHeight; y++) {
        for (uint8_t x = 0; x < fontWidth; x++) {
            uint8_t row = charData[y];
            uint8_t isSet = row & (1 << (fontWidth - 1 - x));
            uint32_t pixelColor = isSet ? color : bgColor;
            
            putPixel(pixelColor, cursorX + x, cursorY + y);
        }
    }
    
    // Mover el cursor a la derecha
    cursorX += fontWidth;
    
    return fontWidth;
}

// Imprime un carácter con los colores por defecto
void vdPrintChar(char character) {
    vdPrintCharStyled(character, defaultFgColor, defaultBgColor);
}

// Imprime una cadena con los colores por defecto
void vdPrint(const char *string) {
    vdPrintStyled((char*)string, defaultFgColor, defaultBgColor);
}

// Imprime una cadena con colores personalizados
void vdPrintStyled(char *s, uint32_t color, uint32_t bgColor) {
    while (*s != 0) {
        vdPrintCharStyled(*s, color, bgColor);
        s++;
    }
}

// Imprime N caracteres de una cadena con colores personalizados
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
    cursorY += (fontStyle == SMALL_FONT) ? HEIGHT_S : HEIGHT_L;
    
    // Verificar si hemos llegado al final de la pantalla
    if (cursorY + ((fontStyle == SMALL_FONT) ? HEIGHT_S : HEIGHT_L) >= SCREEN_HEIGHT_PIXELS) {
        vdClear(); // Por simplicidad, limpiamos la pantalla si llegamos al final
    }
}

// Imprime un número decimal
void vdPrintDec(uint64_t value) {
    char buffer[20];
    int i = 0;
    
    if (value == 0) {
        vdPrintChar('0');
        return;
    }
    
    while (value > 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }
    
    // Imprimir en orden inverso
    while (i > 0) {
        vdPrintChar(buffer[--i]);
    }
}

// Imprime un número en hexadecimal
void vdPrintHex(uint64_t value) {
    char buffer[20];
    int i = 0;
    
    if (value == 0) {
        vdPrint("0x0");
        return;
    }
    
    vdPrint("0x");
    
    while (value > 0) {
        uint8_t digit = value % 16;
        if (digit < 10)
            buffer[i++] = '0' + digit;
        else
            buffer[i++] = 'A' + digit - 10;
        value /= 16;
    }
    
    // Imprimir en orden inverso
    while (i > 0) {
        vdPrintChar(buffer[--i]);
    }
}

// Imprime un número en binario
void vdPrintBin(uint64_t value) {
    char buffer[65];
    int i = 0;
    
    if (value == 0) {
        vdPrint("0b0");
        return;
    }
    
    vdPrint("0b");
    
    while (value > 0) {
        buffer[i++] = '0' + (value % 2);
        value /= 2;
    }
    
    // Imprimir en orden inverso
    while (i > 0) {
        vdPrintChar(buffer[--i]);
    }
}

// Imprime un número en una base específica
void vdPrintBase(uint64_t value, uint32_t base) {
    switch (base) {
        case 10: vdPrintDec(value); break;
        case 16: vdPrintHex(value); break;
        case 2:  vdPrintBin(value); break;
        default: vdPrint("Base no soportada"); break;
    }
}

// Limpia la pantalla
void vdClear() {
    drawRectangle(defaultBgColor, 0, 0, SCREEN_WIDTH_PIXELS, SCREEN_HEIGHT_PIXELS);
    cursorX = 0;
    cursorY = 0;
}

// Elimina el carácter anterior y retrocede el cursor
void vdDelete() {
    uint8_t fontWidth = (fontStyle == SMALL_FONT) ? WIDTH_S : WIDTH_L;
    uint8_t fontHeight = (fontStyle == SMALL_FONT) ? HEIGHT_S : HEIGHT_L;
    
    if (cursorX >= fontWidth) {
        cursorX -= fontWidth;
        
        // Borrar el carácter anterior dibujando un rectángulo con el color de fondo
        for (uint8_t y = 0; y < fontHeight; y++) {
            for (uint8_t x = 0; x < fontWidth; x++) {
                putPixel(defaultBgColor, cursorX + x, cursorY + y);
            }
        }
    }
}

// Cambia el tamaño de la fuente
void vdChangeFontSize() {
    fontStyle = (fontStyle == SMALL_FONT) ? LARGE_FONT : SMALL_FONT;
}

// Dibuja un rectángulo con un color específico
void drawRectangle(uint32_t color, uint16_t up_l_x, uint16_t up_l_y, uint16_t lo_r_x, uint16_t lo_r_y) {
    for (uint16_t y = up_l_y; y < lo_r_y; y++) {
        for (uint16_t x = up_l_x; x < lo_r_x; x++) {
            putPixel(color, x, y);
        }
    }
}