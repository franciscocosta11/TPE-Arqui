#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

// Manejadores de interrupciones externos (implementados en assembler)

/**
 * @brief Manejador de interrupción del timer (IRQ 0)
 * @note Implementado en assembler
 */
extern void _irq00Handler(void);

/**
 * @brief Manejador de interrupción del teclado (IRQ 1)
 * @note Implementado en assembler
 */
extern void _irq01Handler(void);

/**
 * @brief Manejador de syscalls del sistema
 * @note Implementado en assembler
 */
extern void _syscallHandler(void);

// Funciones de control de interrupciones

/**
 * @brief Deshabilita las interrupciones (Clear Interrupt Flag)
 * @note Implementado en assembler - ejecuta CLI
 */
void _cli(void);

/**
 * @brief Habilita las interrupciones (Set Interrupt Flag)
 * @note Implementado en assembler - ejecuta STI
 */
void _sti(void);

/**
 * @brief Detiene el CPU hasta la próxima interrupción
 * @note Implementado en assembler - ejecuta HLT
 */
void _hlt(void);

/**
 * @brief Configura la máscara del PIC maestro
 * @param mask Máscara de 8 bits para habilitar/deshabilitar IRQs 0-7
 * @note Bit en 0 = IRQ habilitado, Bit en 1 = IRQ deshabilitado
 */
void picMasterMask(uint8_t mask);

/**
 * @brief Configura la máscara del PIC esclavo
 * @param mask Máscara de 8 bits para habilitar/deshabilitar IRQs 8-15
 * @note Bit en 0 = IRQ habilitado, Bit en 1 = IRQ deshabilitado
 */
void picSlaveMask(uint8_t mask);

// Funciones de entrada/salida de puertos

/**
 * @brief Lee un byte de un puerto de entrada/salida
 * @param port Número del puerto a leer
 * @return Valor de 8 bits leído del puerto
 */
uint8_t inb(uint16_t port);

/**
 * @brief Escribe un byte a un puerto de entrada/salida
 * @param port Número del puerto donde escribir
 * @param value Valor de 8 bits a escribir
 */
void outb(uint16_t port, uint8_t value);

#endif  // INTERRUPTS_H