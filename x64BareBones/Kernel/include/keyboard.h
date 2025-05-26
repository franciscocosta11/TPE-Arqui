#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

/* Debe ser invocado desde tu stub de IRQ1 */
void keyboard_irq_handler(void);
/* Habilita la IRQ1 en el PIC maestro */
void keyboard_init(void);
/* Bloqueante: devuelve el próximo carácter tecleado */
char keyboard_getchar(void);

#endif // KEYBOARD_H
