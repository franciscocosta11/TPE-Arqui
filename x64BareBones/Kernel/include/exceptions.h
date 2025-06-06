#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdint.h>

/**
 * @brief Dispatcher principal de excepciones del sistema
 * @param exception Número de la excepción que ocurrió
 * @param stack_frame Puntero al frame del stack en el momento de la excepción
 */
void exceptionDispatcher(uint64_t exception, uint64_t *stack_frame);

/**
 * @brief Genera una excepción de división por cero para testing
 * @note Función de prueba - causa una excepción controlada
 */
void triggerDivisionByZero(void);

#endif