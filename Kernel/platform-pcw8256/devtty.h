#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

void tty_irq(void);
int pcwtty_close(uint_fast8_t minor);

#define KEY_ROWS	12
#define KEY_COLS	8
extern uint8_t keymap[12];
extern const uint8_t keyboard[12][8];
extern const uint8_t shiftkeyboard[12][8];

#endif
