#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

#define KEY_COLS	11
#define KEY_ROWS	8
extern uint8_t keymap[11];
extern uint8_t keyboard[11][8];
extern uint8_t shiftkeyboard[11][8];

extern void kbd_interrupt(void);

#endif
