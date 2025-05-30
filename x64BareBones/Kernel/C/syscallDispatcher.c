#include <stdint.h>
#include <videoDriver.h>
#include <keyboard.h>
#include <time.h>

extern void saveRegisters(uint64_t* regs);

void syscallDispatcher(uint64_t rax, uint64_t rdi, uint64_t rsi, uint64_t rdx) {
    switch (rax) {
        case 0: // SYS_READ - lee un carácter del teclado
            {
                char* buffer = (char*)rdi;
                if (buffer != 0) {
                    *buffer = keyboard_getchar();
                }
            }
            break;
            
        case 1: // SYS_WRITE 
            {
                char* buffer = (char*)rdi;
                while (buffer != 0 && *buffer) {
                    vdPrintChar(*buffer);
                    buffer++;
                }
            }
            break;
            
        case 2: // SYS_CLEAR - limpia la pantalla
            vdClear();  // Llama a la función de limpieza del video driver
            break;
        case 3: // SYS_TIME - obtiene la hora del sistema
            {
                char* buffer = (char*)rdi;
                if (buffer != 0) {
                    getSystemTime(buffer);
                }
            }
            break;
            
        case 4: // SYS_REGISTERS - obtiene los valores de los registros
            {
                uint64_t* regs = (uint64_t*)rdi;
                if (regs != 0) {
                    saveRegisters(regs);
                }
            }
            break;
            
        case 5: // SYS_EXCEPTION - genera intencionalmente una excepción
            {
                int exceptionNumber = (int)rdi;
                if (exceptionNumber == 0) {
                    // División por cero
                    int zero = 0;
                    int result = 1 / zero;
                    (void)result; // Para evitar warning de variable no usada
                }
                else if (exceptionNumber == 1) {
                    // Acceso a memoria inválida
                    char *ptr = (char*)0x0;
                    *ptr = 'X';
                }
            }
            break;
        case 6: // SYS_FONT_SIZE - cambia el tamaño de la fuente
            {
                uint8_t size = (uint8_t)rdi;
                vdSetFontSize(size);
            }
            break;
    }
}