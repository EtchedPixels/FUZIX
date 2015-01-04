#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

void tty_putc(uint8_t minor, char c);
bool tty_writeready(uint8_t minor);
void tty_init_port(void);
void tty_irq(void);

#define KEY_ROWS	12
#define KEY_COLS	8
extern uint8_t keymap[12];
extern uint8_t keyboard[12][8];
extern uint8_t shiftkeyboard[12][8];

#endif
