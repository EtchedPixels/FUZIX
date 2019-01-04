#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

void tty_pollirq(void);
static void keydecode(void);

extern void tty_polluart(void);

#define KEY_ROWS	8
#define KEY_COLS	5
extern uint8_t keymap[8];
extern uint8_t keyboard[8][5];
extern uint8_t shiftkeyboard[8][5];

extern uint8_t timer_wait;

extern int gfx_ioctl(uint8_t minor, uarg_t arg, char *ptr);

extern uint8_t vtborder;

#endif
