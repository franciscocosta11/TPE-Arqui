#include "soundDriver.h"

// Funciones para acceso a puertos
extern void outb(uint16_t port, uint8_t value);
extern uint8_t inb(uint16_t port);

// Variable para controlar el estado del speaker
static uint8_t speaker_enabled = 1;

void soundOff(void) {
    // Desactivar el speaker
    uint8_t speaker_status = inb(SPEAKER_PORT);
    outb(SPEAKER_PORT, speaker_status & 0xFC);
}

void playSound(uint32_t frequency, uint32_t duration_ms) {
    if (!speaker_enabled || frequency == 0) {
        return;
    }
    
    // Calcular divisor para la frecuencia
    uint32_t divisor = 1193180 / frequency;
    
    // Configurar PIT
    outb(PIT_COMMAND_PORT, 0xB6);
    outb(PIT_CHANNEL_2, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL_2, (uint8_t)(divisor >> 8));
    
    // Activar speaker
    uint8_t speaker_status = inb(SPEAKER_PORT);
    outb(SPEAKER_PORT, speaker_status | 0x03);
    
    // Delay
    for (volatile uint32_t i = 0; i < duration_ms * 2000; i++) {
        for (volatile int j = 0; j < 100; j++);
    }
    
    // Apagar speaker
    soundOff();
}

void soundInit(void) {
    speaker_enabled = 1;
    soundOff(); 
    
    // Sonido de bienvenida al sistema
    playSound(523, 200);  // C5
    playSound(659, 200);  // E5
    playSound(784, 300);  // G5
}

void playBeep(void) {
    playSound(800, 100);
}

void playWinSound(void) {
    playSound(523, 150);  // C5
    playSound(659, 150);  // E5
    playSound(784, 150);  // G5
    playSound(1047, 300); // C6
}