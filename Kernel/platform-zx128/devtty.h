#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

void tty_pollirq(void);
static void keydecode(void);

#define KEY_ROWS	8
#define KEY_COLS	5
extern uint8_t keymap[8];
extern uint8_t keyboard[8][5];
extern uint8_t shiftkeyboard[8][5];

#endif
