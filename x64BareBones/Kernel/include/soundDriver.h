#ifndef SOUND_DRIVER_H
#define SOUND_DRIVER_H

#include <stdint.h>

// Definiciones del speaker del PC
#define PIT_COMMAND_PORT    0x43
#define PIT_CHANNEL_2       0x42
#define SPEAKER_PORT        0x61

// Frecuencias musicales (en Hz)
#define NOTE_C4     262
#define NOTE_D4     294
#define NOTE_E4     330
#define NOTE_F4     349
#define NOTE_G4     392
#define NOTE_A4     440
#define NOTE_B4     494
#define NOTE_C5     523
#define NOTE_D5     587
#define NOTE_E5     659

// Funciones del driver de sonido

/**
 * @brief Inicializa el driver de sonido del sistema
 * @note Configura el PIT y el speaker del PC para reproducir sonidos
 */
void soundInit(void);

/**
 * @brief Reproduce un beep simple y corto
 * @note Sonido estándar del sistema, frecuencia y duración predefinidas
 */
void playBeep(void);

/**
 * @brief Reproduce una melodía de victoria
 * @note Secuencia de tonos ascendentes para indicar éxito
 */
void playWinSound(void);

/**
 * @brief Reproduce un sonido con frecuencia y duración específicas
 * @param frequency Frecuencia del sonido en Hz (20-20000 Hz recomendado)
 * @param duration_ms Duración del sonido en milisegundos
 */
void playSound(uint32_t frequency, uint32_t duration_ms);

/**
 * @brief Apaga el speaker y detiene cualquier sonido en reproducción
 * @note Silencia inmediatamente el speaker del PC
 */
void soundOff(void);

/**
 * @brief Reproduce un sonido de error
 * @note Tono grave y distintivo para indicar errores
 */
void playErrorSound(void);

/**
 * @brief Reproduce un sonido de click/confirmación
 * @note Sonido breve para confirmación de acciones de usuario
 */
void playClickSound(void);

#endif