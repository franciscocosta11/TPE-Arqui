#include <keyboard.h>
#include <interrupts.h>   // picMasterMask
#include <stdint.h>

static inline uint8_t inb(uint16_t port) {
    uint8_t b;
    __asm__ volatile ("inb %1, %0" : "=a"(b) : "Nd"(port));
    return b;
}
static inline void outb(uint16_t port, uint8_t v) {
    __asm__ volatile ("outb %0, %1" : : "a"(v), "Nd"(port));
}

/* Mapa de scancodes (set1), sin Shift ni Control */
static const char keymap[128] = {
    [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4',
    [0x06] = '5', [0x07] = '6', [0x08] = '7', [0x09] = '8',
    [0x0A] = '9', [0x0B] = '0', [0x0C] = '-', [0x0D] = '=',
    [0x0E] = '\b',[0x0F] = '\t',
    [0x10] = 'q',[0x11] = 'w',[0x12] = 'e',[0x13] = 'r',
    [0x14] = 't',[0x15] = 'y',[0x16] = 'u',[0x17] = 'i',
    [0x18] = 'o',[0x19] = 'p',[0x1A] = '[',[0x1B] = ']',
    [0x1C] = '\n',
    [0x1E] = 'a',[0x1F] = 's',[0x20] = 'd',[0x21] = 'f',
    [0x22] = 'g',[0x23] = 'h',[0x24] = 'j',[0x25] = 'k',
    [0x26] = 'l',[0x27] = ';',[0x28] = '\'',[0x29] = '`',
    [0x2C] = 'z',[0x2D] = 'x',[0x2E] = 'c',[0x2F] = 'v',
    [0x30] = 'b',[0x31] = 'n',[0x32] = 'm',[0x33] = ',',
    [0x34] = '.',[0x35] = '/',
    [0x39] = ' '
};

static volatile char buffer = 0;
static volatile int  ready  = 0;

void keyboard_irq_handler(void) {
    uint8_t sc = inb(0x60);
    if (sc < 128) {
        char c = keymap[sc];
        if (c) {
            buffer = c;
            ready  = 1;
        }
    }
    outb(0x20, 0x20);  // EOI al PIC maestro
}

void keyboard_init(void) {
    picMasterMask(0xFC);  // 11111100: habilita IRQ0 (timer) e IRQ1 (teclado)
}

char keyboard_getchar(void) {
    while (!ready) { __asm__ volatile ("hlt"); }
    ready = 0;
    return buffer;
}
