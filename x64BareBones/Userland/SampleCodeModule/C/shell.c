#include "./../include/libc.h"
#define CMD_MAX 32
static uint8_t currentFontSize = 1;

void displayHelp() {
    print("Comandos disponibles:\n");
    print("  ls      - Lista todos los comandos disponibles\n");
    print("  help    - Muestra esta ayuda\n");
    print("  clear   - Limpia la pantalla\n");
    print("  time    - Muestra la hora actual del sistema\n");
    print("  ex1     - Genera una excepción de división por cero\n");
    print("  ex2     - Genera una excepción de acceso inválido a memoria\n");
    print("  regs    - Muestra el valor de todos los registros del procesador\n");
    print("  font    - Cambia el tamaño de la fuente (alterna entre pequeño y grande)\n");
}

void displayRegisters() {
    uint64_t regs[19];
    getRegisters(regs);
    
    char buffer[32];
    const char *regNames[] = {
        "RAX", "RBX", "RCX", "RDX", "RSI", "RDI", "RSP", "RBP",
        "R8 ", "R9 ", "R10", "R11", "R12", "R13", "R14", "R15",
        "RIP", "CS ", "RFLAGS"
    };
    
    print("Valores de los registros:\n");
    for (int i = 0; i < 19; i++) {
        print("  ");
        print(regNames[i]);
        print(" = 0x");
        
        // Convertir valor a hexadecimal (simplificado)
        char hexChars[] = "0123456789ABCDEF";
        for (int j = 15; j >= 0; j--) {
            buffer[15-j] = hexChars[(regs[i] >> (j*4)) & 0xF];
        }
        buffer[16] = '\0';
        print(buffer);
        print("\n");
    }
}

void shell() {
    char cmd[CMD_MAX];
    int cmdLen = 0;
    int promptLength = 19; // Longitud de "@mark_zuckerberg$> " asi no me lo borran...

    print("@mark_zuckerberg$> ");

    while (1) {
        char c = getchar();

        // Manejar el backspace
        if (c == '\b') {
            if (cmdLen > 0) {  // Solo borrar si hay caracteres en el buffer
                cmdLen--;      // Decrementar el contador de caracteres
                putchar(c);    // Imprimir el backspace (eco)
            }
        }
        // Acumular caracteres hasta '\n' (no incluye backspace)
        else if (c != '\n' && cmdLen < CMD_MAX-1) {
            putchar(c);
            cmd[cmdLen++] = c;
        }
        else if (c == '\n') {
            putchar(c);             // eco del '\n'
            cmd[cmdLen] = '\0';     // cierre de cadena

            // Procesar comando
            if (strcmp(cmd, "ls") == 0) {
                print("Comandos disponibles: ls, help, ex1, ex2, time, regs, clear, font\n");
            }
            else if (strcmp(cmd, "help") == 0) {
                displayHelp();
            }
            else if (strcmp(cmd, "clear") == 0) {
                clearScreen();
            }
            else if (strcmp(cmd, "ex1") == 0) {
                print("[ex1] Ejecutando excepcion de división por cero...\n");
                triggerException(0);
                print("[ex1] La excepción no fue capturada correctamente\n");
            }
            else if (strcmp(cmd, "ex2") == 0) {
                print("[ex2] Ejecutando excepcion de acceso a memoria invalida...\n");
                triggerException(1);
                print("[ex2] La excepcion no fue capturada correctamente\n");
            }
            else if (strcmp(cmd, "time") == 0) {
                char timeBuffer[20];
                getTime(timeBuffer);
                print("[time] Hora actual: ");
                print(timeBuffer);
                print("\n");
            }
            else if (strcmp(cmd, "regs") == 0) {
                displayRegisters();
            }
            else if (strcmp(cmd, "font") == 0) {
                // Alternar entre tamaños: 1 (pequeño), 2 (mediano) y 3 (grande)
                currentFontSize = (currentFontSize % 3) + 1;
                
                // limpio la pantalla por si las moscas
                clearScreen();
                
                // Cambiar el tamaño de la fuente!
                setFontSize(currentFontSize);
                
                // se lo cuento al usuario asi se rescata
                print("Tamanio de fuente cambiado a ");
                if (currentFontSize == 1) print("pequenio");
                else if (currentFontSize == 2) print("mediano");
                else print("grande");
                print("\n");
            }
            else if (cmdLen > 0) {
                print("Comando no encontrado\n");
            }
            else {
                print("Comando vacio, por favor ingresa un comando.\n");
            }

            // Reiniciar buffer y mostrar prompt
            cmdLen = 0;
            print("@mark_zuckerberg$> ");
        }
    }
}