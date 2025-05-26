#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

/* --- Stubs de excepción e IRQ definidos en interrupts.asm --- */
extern void _exception0Handler(void);
extern void _exception06Handler(void);
extern void _irq00Handler(void);
extern void _irq01Handler(void);
extern void _syscallHandler(void);

/* --- Funciones para PIC y control de interrupciones --- */
void picMasterMask(uint8_t mask);
void picSlaveMask(uint8_t mask);
void sti(void);

#endif  // INTERRUPTS_H
