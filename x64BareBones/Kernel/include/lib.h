#ifndef LIB_H
#define LIB_H

#include <stdint.h>

/**
 * @brief Llena un bloque de memoria con un valor específico
 * @param destination Puntero al bloque de memoria a llenar
 * @param character Valor a escribir en cada byte (convertido a unsigned char)
 * @param length Número de bytes a llenar
 * @return Puntero al bloque de memoria modificado
 */
void * memset(void * destination, int32_t character, uint64_t length);

/**
 * @brief Copia un bloque de memoria de origen a destino
 * @param destination Puntero al área de memoria destino
 * @param source Puntero al área de memoria origen
 * @param length Número de bytes a copiar
 * @return Puntero al área de memoria destino
 */
void * memcpy(void * destination, const void * source, uint64_t length);

/**
 * @brief Obtiene la información del fabricante del CPU
 * @param result Buffer donde se almacenará el string del fabricante
 * @return Puntero al buffer con el string del fabricante del CPU
 * @note El buffer debe tener al menos 13 bytes (12 caracteres + '\0')
 */
char *cpuVendor(char *result);

#endif