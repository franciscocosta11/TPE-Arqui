#ifndef KEYBOARD_H
#define KEYBOARD_H

/**
 * @brief Manejador de interrupción del teclado
 * @note Procesa las teclas presionadas y las almacena en el buffer interno
 */
void keyboard_irq_handler(void);

/**
 * @brief Inicializa el subsistema del teclado
 * @note Configura el controlador de teclado y limpia el buffer
 */
void keyboard_init(void);

/**
 * @brief Lee un carácter del buffer del teclado (bloqueante)
 * @return Carácter leído del teclado
 * @note Bloquea hasta que haya un carácter disponible
 */
char keyboard_getchar(void);

/**
 * @brief Limpia el buffer interno del teclado
 * @note Descarta todos los caracteres pendientes en el buffer
 */
void keyboard_clear_buffer(void);

/**
 * @brief Lee un carácter del buffer del teclado (no bloqueante)
 * @return Carácter leído del teclado, o 0 si no hay caracteres disponibles
 * @note No bloquea si no hay caracteres en el buffer
 */
char keyboard_getchar_nonblocking(void);

/**
 * @brief Verifica si hay teclas disponibles en el buffer
 * @return 1 si hay al menos una tecla en el buffer, 0 en caso contrario
 * @note Útil para implementar entrada no bloqueante
 */
int keyboard_has_key(void);

#endif