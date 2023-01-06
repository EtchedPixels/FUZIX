#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

void tty_pollirq(unsigned irq);
static void keydecode(void);

#define KEY_ROWS	8
#define KEY_COLS	6
extern uint8_t keymap[8];
extern uint8_t keyboard[8][6];
extern uint8_t shiftkeyboard[8][6];

#endif
