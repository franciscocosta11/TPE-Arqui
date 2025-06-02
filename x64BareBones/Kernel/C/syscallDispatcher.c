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
            
        case 6: // SYS_FONT_SIZE
            {
                uint8_t size = (uint8_t)rsi;  // CAMBIADO: era rdi, ahora rsi
                vdSetFontSize(size);
            }
            break;
            
        case 7: // SYS_SET_COLOR
            {
         
                char* colorName = (char*)rsi;  // CAMBIADO: era rdi, ahora rsi
                if (colorName != 0) {
                    uint32_t color = vdGetColorByName(colorName);
                    vdSetColor(color);
                    vdPrint("Color cambiado correctamente!\n");
                }
            }
            break;
            
        // default:
        //     vdPrint("SYSCALL DESCONOCIDO\n");
        //     break;
    }
}