#ifndef _DEVTTY_H
#define _DEVTTY_H

extern void tty_interrupt(void);
extern void kbd_interrupt(void);
extern int trstty_close(uint8_t minor);
extern void vtbuf_init(void);

#define KEY_ROWS	8
#define KEY_COLS	8
extern uint8_t keymap[8];
extern uint8_t keyboard[8][8];
extern uint8_t shiftkeyboard[8][8];

extern uint8_t *vtbase[2];
extern uint8_t curtty;

extern uint8_t gfxtype;
#endif
