#include <keyboard.h>
#include <interrupts.h>   
#include <stdint.h>
#include <videoDriver.h>

#define CAPS_LOCK       0x3A
#define CAPS_LOCK_REL   0xBA
#define LSHIFT          0x2A
#define RSHIFT          0x36
#define LSHIFT_REL      0xAA
#define RSHIFT_REL      0xB6

extern uint64_t capture_provisoria[19];
extern uint64_t capture_definitiva[19];

static int shift_pressed = 0;
static int caps_lock_on = 0; 

static const char keymap[128] = {
    [0x48] = 72,  [0x50] = 80,  [0x4D] = 77,  [0x4B] = 75,
    [0x01] = 27,  [0x02] = '1', [0x03] = '2', [0x04] = '3',
    [0x05] = '4', [0x06] = '5', [0x07] = '6', [0x08] = '7',
    [0x09] = '8', [0x0A] = '9', [0x0B] = '0', [0x0C] = '-', [0x0D] = '=',
    [0x0E] = '\b',[0x0F] = '\t',
    [0x10] = 'q',[0x11] = 'w',[0x12] = 'e',[0x13] = 'r',
    [0x14] = 't',[0x15] = 'y',[0x16] = 'u',[0x17] = 'i',
    [0x18] = 'o',[0x19] = 'p',[0x1A] = '[',[0x1B] = ']',
    [0x1C] = '\n',
    [0x1E] = 'a',[0x1F] = 's',[0x20] = 'd',[0x21] = 'f',
    [0x22] = 'g',[0x23] = 'h',[0x24] = 'j',[0x25] = 'k',
    [0x26] = 'l',[0x27] = ';',[0x28] = '\'',[0x29] = '`',
    [0x2B] = '\\',
    [0x2C] = 'z',[0x2D] = 'x',[0x2E] = 'c',[0x2F] = 'v',
    [0x30] = 'b',[0x31] = 'n',[0x32] = 'm',[0x33] = ',',
    [0x34] = '.',[0x35] = '/',
    [0x39] = ' '
};

static const char keymap_shift[128] = {
    [0x02] = '!', [0x03] = '@', [0x04] = '#', [0x05] = '$',
    [0x06] = '%', [0x07] = '^', [0x08] = '&', [0x09] = '*',
    [0x0A] = '(', [0x0B] = ')', [0x0C] = '_', [0x0D] = '+',
    [0x0E] = '\b',[0x0F] = '\t',
    [0x10] = 'Q',[0x11] = 'W',[0x12] = 'E',[0x13] = 'R',
    [0x14] = 'T',[0x15] = 'Y',[0x16] = 'U',[0x17] = 'I',
    [0x18] = 'O',[0x19] = 'P',[0x1A] = '{',[0x1B] = '}',
    [0x1C] = '\n',
    [0x1E] = 'A',[0x1F] = 'S',[0x20] = 'D',[0x21] = 'F',
    [0x22] = 'G',[0x23] = 'H',[0x24] = 'J',[0x25] = 'K',
    [0x26] = 'L',[0x27] = ':',[0x28] = '"',[0x29] = '~',
    [0x2B] = '|',
    [0x2C] = 'Z',[0x2D] = 'X',[0x2E] = 'C',[0x2F] = 'V',
    [0x30] = 'B',[0x31] = 'N',[0x32] = 'M',[0x33] = '<',
    [0x34] = '>',[0x35] = '?',
    [0x39] = ' '
};

static volatile char buffer = 0;
static volatile int  ready  = 0;

void keyboard_irq_handler(void) {
    uint8_t sc = inb(0x60);

    // Manejo de teclas especiales
    if (sc == LSHIFT || sc == RSHIFT) {
        shift_pressed = 1;
        return;
    } else if (sc == LSHIFT_REL || sc == RSHIFT_REL) {
        shift_pressed = 0;
        return;
    }

    if (sc == CAPS_LOCK) {
        caps_lock_on = !caps_lock_on;
        return;
    } else if (sc == CAPS_LOCK_REL) {
        return;
    }

    // Captura de registros al presionar '9' (scancode 0x0A)
    if (sc == 0x0A) {
        for (int i = 0; i < 19; i++) {
            capture_definitiva[i] = capture_provisoria[i];
        }
        return;
    }

    // Ignorar teclas liberadas
    if (sc & 0x80) return;

    // Traducción de scancode a carácter
    char c = 0;
    if ((sc >= 0x10 && sc <= 0x19) || (sc >= 0x1E && sc <= 0x26) || (sc >= 0x2C && sc <= 0x32)) {
        if ((shift_pressed && !caps_lock_on) || (!shift_pressed && caps_lock_on)) {
            c = keymap_shift[sc];
        } else {
            c = keymap[sc];
        }
    } else {
        c = shift_pressed ? keymap_shift[sc] : keymap[sc];
    }

    // Guardar en buffer si es válido
    if (c) {
        buffer = c;
        ready = 1;
    }
}

void keyboard_init(void) {
    picMasterMask(0xFC); 
}

char keyboard_getchar(void) {
    while (!ready) {
        _hlt();
    }
    ready = 0;
    return buffer;
}

void keyboard_clear_buffer(void) {
    ready = 0;
    buffer = 0;
    while (inb(0x64) & 0x01) {
        inb(0x60);  // limpiar el buffer del hardware
    }
}

int keyboard_has_key(void) {
    return ready;
}

char keyboard_getchar_nonblocking(void) {
    if (ready) {
        ready = 0;
        return buffer;
    }
    return 0;
}
