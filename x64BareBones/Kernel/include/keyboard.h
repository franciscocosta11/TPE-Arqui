#ifndef KEYBOARD_H
#define KEYBOARD_H

void keyboard_irq_handler(void);
void keyboard_init(void);
char keyboard_getchar(void);
void keyboard_clear_buffer(void);

char keyboard_getchar_nonblocking(void);
int keyboard_has_key(void) ;

#endif