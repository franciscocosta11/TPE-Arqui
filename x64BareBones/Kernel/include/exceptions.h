#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdint.h>

// Función principal del dispatcher de excepciones
void exceptionDispatcher(uint64_t exception, uint64_t *stack_frame);

// Funciones para test de excepciones
void triggerDivisionByZero(void);
void triggerInvalidOpcode(void);

#endif