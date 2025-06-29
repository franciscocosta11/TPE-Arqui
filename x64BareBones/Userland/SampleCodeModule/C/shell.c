#include "./../include/libc.h"
#include "./../include/shell.h"
#define USER_LENGTH 19
#define CMD_MAX 64
#define USERNAME_MAX 32
static uint8_t currentFontSize = 1;

static char username[USERNAME_MAX] = "mark_zuckerberg";

void startGolfGame(void);

extern void triggerInvalidOpcode(void);

static void triggerDivisionByZero(void) {
    volatile int a = 1;
    volatile int b = 0;
    volatile int result = a / b;
    (void)result;
}

void printPrompt() {
    print("@");
    print(username);
    print("$> ");
}

int parseCommand(char* input, char* command, char* argument) {
    int i = 0, j = 0;
    
    while (input[i] != ' ' && input[i] != '\0' && j < CMD_MAX - 1) {
        command[j] = input[i];
        i++;
        j++;
    }
    command[j] = '\0';
    
    while (input[i] == ' ') {
        i++;
    }
    
    j = 0;
    while (input[i] != '\0' && j < USERNAME_MAX - 1) {
        argument[j] = input[i];
        i++;
        j++;
    }
    argument[j] = '\0';
    
    return (argument[0] != '\0') ? 1 : 0;
}

void displayHelp() {
    print("Comandos disponibles:\n");
    print("  ls       - Lista todos los comandos disponibles\n");
    print("  help     - Muestra esta ayuda\n");
    print("  clear    - Limpia la pantalla\n");
    print("  time     - Muestra la hora actual del sistema\n");
    print("  ex1      - Genera una excepcion de division por cero\n");
    print("  ex2      - Genera una excepcion de opcode invalido\n");
    print("  regs     - Muestra el valor de todos los registros del procesador\n");
    print("  font     - Cambia el tamanio de la fuente\n");
    print("  color <nombre> - Cambia el color del texto\n");
    print("  user <nombre>  - Cambia el nombre de usuario\n");
    print("  golf     - Inicia el juego Pongis-Golf\n");
    print("  9        - Captura de registros \n");
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
    char input[CMD_MAX];
    char command[CMD_MAX];
    char argument[USERNAME_MAX];
    int inputLen = 0;

    print("Bienvenido al mejor tpe de Arqui!\n");
    
    printPrompt();

    while (1) {
        char c = getchar();

        if (c == '\b') {
            if (inputLen > 0) {
                inputLen--;
                putchar(c);
            }
        }
        else if (c != '\n' && inputLen < CMD_MAX-1) {
            putchar(c);
            input[inputLen++] = c;
        }
        else if (c == '\n') {
            putchar(c);
            input[inputLen] = '\0';

        
            int hasArgument = parseCommand(input, command, argument);

            if (strcmp(command, "ls") == 0) {
                print("Comandos disponibles: ls, help, ex1, ex2, time, regs, clear, font, color, user, golf\n");
            }
            else if (strcmp(command, "help") == 0) {
                displayHelp();
            }
            else if (strcmp(command, "clear") == 0) {
                clearScreen();
            }
            else if (strcmp(command, "ex1") == 0) {
                print("[ex1] Ejecutando excepcion de division por cero...\n");
                triggerDivisionByZero();
            }
            else if (strcmp(command, "ex2") == 0) {
                print("[ex2] Ejecutando excepcion de opcode invalido...\n");
                triggerInvalidOpcode();
            }
            else if (strcmp(command, "time") == 0) {
                char timeBuffer[20];
                getTime(timeBuffer);
                print("[time] Hora actual: ");
                print(timeBuffer);
                print("\n");
            }
            else if (strcmp(command, "regs") == 0) {
                displayRegisters();
            }
            else if (strcmp(command, "font") == 0) {
                uint8_t newSize = (currentFontSize % 3) + 1;
                
                int result = syscall(5, newSize, 0, 0);
                
                if (result == 0) {
                    currentFontSize = newSize;
                    clearScreen();
                    print("Tamanio de fuente cambiado a ");
                    if (currentFontSize == 1) print("pequenio");
                    else if (currentFontSize == 2) print("mediano");
                    else print("grande");
                    print("\n");
                } else {
                    print("Error: No se pudo cambiar el tamaño de fuente en este hardware\n");
                }
            }
            else if (strcmp(command, "color") == 0) {
                if (hasArgument) {
                    if (setTextColor(argument)) {
                        print("Color cambiado a: ");
                        print(argument);
                        print("\n");
                    } else {
                        print("Color no reconocido. ");
                        printAvailableColors();
                    }
                } else {
                    print("Uso: color <nombre>\n");
                    printAvailableColors();
                }
            }
            else if (strcmp(command, "user") == 0) {
                if (hasArgument) {
                    int len = 0;
                    while (argument[len] != '\0' && len < USERNAME_MAX - 1) len++;
                    
                    if (len > 0 && len < USERNAME_MAX - 1) {
                        for (int i = 0; i <= len; i++) {
                            username[i] = argument[i];
                        }
                        print("Nombre de usuario cambiado a: ");
                        print(username);
                        print("\n");
                    } else {
                        print("Nombre de usuario demasiado largo\n");
                    }
                } else {
                    print("Uso: user <nombre>\n");
                    print("Usuario actual: ");
                    print(username);
                    print("\n");
                }
            }
            else if (strcmp(command, "golf") == 0) {
                clearScreen();
                playBeep();
                startGolfGame();
                clearScreen();
                print("Gracias por jugar Pongis-Golf!\n");
            }
            else if (inputLen > 0) {
                print("Comando no encontrado: ");
                print(command);
                print("\n");
            }

            inputLen = 0;
            printPrompt();
        }
    }
}