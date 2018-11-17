#ifndef _DEVTTY_H
#define _DEVTTY_H

extern void tty_interrupt(void);
extern void kbd_interrupt(void);
extern void keyscan(void);

#define KEY_ROWS	9
#define KEY_COLS	8

extern uint8_t keymap[9];
extern uint8_t keyboard[9][8];
extern uint8_t shiftkeyboard[9][8];

#endif
