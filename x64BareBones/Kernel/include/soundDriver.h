#ifndef SOUND_DRIVER_H
#define SOUND_DRIVER_H

#include <stdint.h>

// Definiciones del speaker del PC
#define PIT_COMMAND_PORT    0x43
#define PIT_CHANNEL_2       0x42
#define SPEAKER_PORT        0x61

// Frecuencias musicales comunes (en Hz)
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
void soundInit(void);
void playBeep(void);
void playWinSound(void);
void playSound(uint32_t frequency, uint32_t duration_ms);
void soundOff(void);
void playErrorSound(void);
void playClickSound(void);

#endif
