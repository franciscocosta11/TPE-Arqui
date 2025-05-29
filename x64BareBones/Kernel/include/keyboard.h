#ifndef KEYBOARD_H
#define KEYBOARD_H

// Inicializa el controlador de teclado
void keyboard_init(void);

// Maneja la interrupción de teclado
void keyboard_irq_handler(void);

// Obtiene un carácter del teclado (espera hasta que haya uno disponible)
char keyboard_getchar(void);


#endif