#include <stdint.h>
#include <videoDriver.h>
#include <keyboard.h>
#include <time.h>

extern void saveRegisters(uint64_t* regs);

void syscallDispatcher(uint64_t rax, uint64_t rsi, uint64_t rdx, uint64_t rcx) {
    switch (rax) {
        case 0: // SYS_READ
            {
                char* buffer = (char*)rsi;  // CAMBIADO: era rdi, ahora rsi
                if (buffer != 0) {
                    *buffer = keyboard_getchar();
                }
            }
            break;
            
        case 1: // SYS_WRITE 
            {
                char* buffer = (char*)rsi;  // CAMBIADO: era rdi, ahora rsi
                while (buffer != 0 && *buffer) {
                    vdPrintChar(*buffer);
                    buffer++;
                }
            }
            break;
            
        case 2: // SYS_CLEAR
            vdClear();
            break;
            
        case 3: // SYS_TIME
            {
                char* buffer = (char*)rsi;  // CAMBIADO: era rdi, ahora rsi
                if (buffer != 0) {
                    getSystemTime(buffer);
                }
            }
            break;
            
        case 4: // SYS_REGISTERS
            {
                uint64_t* regs = (uint64_t*)rsi;  // CAMBIADO: era rdi, ahora rsi
                if (regs != 0) {
                    saveRegisters(regs);
                }
            }
            break;
            
        case 5: // SYS_FONT_SIZE
            {
                uint8_t size = (uint8_t)rsi;  // CAMBIADO: era rdi, ahora rsi
                vdSetFontSize(size);
            }
            break;
            
        case 6: // SYS_SET_COLOR
            {
         
                char* colorName = (char*)rsi;  // CAMBIADO: era rdi, ahora rsi
                if (colorName != 0) {
                    uint32_t color = vdGetColorByName(colorName);
                    vdSetColor(color);
                    vdPrint("Color cambiado correctamente!\n");
                }
            }
            break;

       
    
    // Agregar estos casos al switch en syscallDispatcher.c

case 8: // SYS_FILL_SCREEN - llenar pantalla con color
    {
        uint32_t color = (uint32_t)rsi;
        vdFillScreen(color);
    }
    break;

case 9: // SYS_GET_KEY_NONBLOCKING
    {
        char* key_ptr = (char*)rsi;  // Recibir puntero
        if (key_ptr && keyboard_has_key()) {
            *key_ptr = keyboard_getchar_nonblocking();
        } else if (key_ptr) {
            *key_ptr = 0;
        }
    }
    break;

case 10: // SYS_PLAY_SOUND - reproducir sonido
    {
        uint32_t frequency = (uint32_t)rsi;
        uint32_t duration = (uint32_t)rdx;
       // playSound(frequency, duration);
    }
    break;
    case 11: // SYS_PUT_PIXEL
    {
        uint32_t color = (uint32_t)rsi;
        uint64_t x = rdx;
        uint64_t y = rcx;
        putPixel(color, x, y);
    }
    break;

case 12: // SYS_DRAW_RECTANGLE  
    {
        uint32_t color = (uint32_t)rsi;
        uint16_t x1 = (uint16_t)rdx;
        uint16_t y1 = (uint16_t)rcx;
        // Necesitamos más parámetros - usar syscall diferente
        drawRectangle(color, x1, y1, x1+100, y1+50); // temporal
    }
    break;
}}