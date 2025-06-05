#include <videoDriver.h>
#include <stdint.h>
#include <font.h>

#ifndef NULL
#define NULL ((void*)0)
#endif

uint8_t currentFontSize = 1;

struct vbe_mode_info_structure {
    uint16_t attributes;
    uint8_t window_a;
    uint8_t window_b;
    uint16_t granularity;
    uint16_t window_size;
    uint16_t segment_a;
    uint16_t segment_b;
    uint32_t win_func_ptr;
    uint16_t pitch;
    uint16_t width;
    uint16_t height;
    uint8_t w_char;
    uint8_t y_char;
    uint8_t planes;
    uint8_t bpp;
    uint8_t banks;
    uint8_t memory_model;
    uint8_t bank_size;
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
    uint32_t framebuffer;
    uint32_t off_screen_mem_off;
    uint16_t off_screen_mem_size;
    uint8_t reserved1[206];
} __attribute__ ((packed));

typedef struct vbe_mode_info_structure * VBEInfoPtr;

VBEInfoPtr VBE_mode_info = (VBEInfoPtr) 0x0000000000005C00;

// Variables globales para manejar el cursor y colores
static uint16_t cursorX = 0;
static uint16_t cursorY = 0;
static uint32_t defaultFgColor = 0xFFFFFF; // Blanco por defecto
static uint32_t defaultBgColor = 0x000000; // Negro por defecto

// AGREGADO: Tabla de colores disponibles
typedef struct {
    const char* name;
    uint32_t color;
} color_entry_t;

static const color_entry_t available_colors[] = {
    {"white", 0xFFFFFF},
    {"red", 0xFF0000},
    {"green", 0x00FF00},
    {"blue", 0x0000FF},
    {"yellow", 0xFFFF00},
    {"cyan", 0x00FFFF},
    {"magenta", 0xFF00FF},
    {"orange", 0xFF8000},
    {"purple", 0x8000FF},
    {"pink", 0xFF80FF},
    {"gray", 0x808080},
    {"lightgray", 0xC0C0C0},
    {"darkgray", 0x404040},
    {NULL, 0} // Terminador
};

static int vd_strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

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

// AGREGADO: Función para cambiar color del texto
// void vdSetColor(uint32_t fgColor) {
//     defaultFgColor = fgColor;
// }

// AGREGADO: Función para obtener color por nombre
uint32_t vdGetColorByName(const char* colorName) {
    for (int i = 0; available_colors[i].name != 0; i++) {
        if (vd_strcmp(colorName, available_colors[i].name) == 0) {  // CAMBIADO: usar vd_strcmp
            return available_colors[i].color;
        }
    }
    return 0xFFFFFF; // Blanco por defecto si no se encuentra
}

// AGREGADO: Función para listar colores disponibles
void vdPrintAvailableColors() {
    vdPrint("Colores disponibles: ");
    for (int i = 0; available_colors[i].name != NULL; i++) {
        vdPrint(available_colors[i].name);
        if (available_colors[i + 1].name != NULL) {
            vdPrint(", ");
        }
    }
    vdPrint("\n");
}

// Resto de funciones existentes (sin cambios)
void vdPrintChar(char character) {
    vdPrintCharStyled(character, defaultFgColor, defaultBgColor);
}

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

void vdNewline() {
    cursorX = 0;
    cursorY += HEIGHT_S * currentFontSize;
    
    if (cursorY + (HEIGHT_S * currentFontSize) >= SCREEN_HEIGHT_PIXELS) {
        vdClear(); 
    }
}

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

// AGREGADO: Función para cambiar color del texto (con debug)
void vdSetColor(uint32_t fgColor) {
    defaultFgColor = fgColor;
    
    // DEBUG: Imprimir confirmación (temporal)
    // Puedes quitar esto después de que funcione
    if (fgColor == 0xFF0000) {
        // Es rojo, debug sin usar el color nuevo todavía
    }
}

// Agregar estas funciones a videoDriver.c

// Función para llenar toda la pantalla con un color
void vdFillScreen(uint32_t color) {
    for (uint16_t y = 0; y < SCREEN_HEIGHT_PIXELS; y++) {
        for (uint16_t x = 0; x < SCREEN_WIDTH_PIXELS; x++) {
            putPixel(color, x, y);
        }
    }
}