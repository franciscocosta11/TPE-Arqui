#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

extern void _irq00Handler(void);
extern void _irq01Handler(void);
extern void _syscallHandler(void);


void _cli(void);
void _sti(void);
void _hlt(void);
void picMasterMask(uint8_t mask);
void picSlaveMask(uint8_t mask);


uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t value);

#endif  // INTERRUPTS_H
