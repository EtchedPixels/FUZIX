#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

extern int nc100_tty_open(uint8_t minor, uint16_t flag);
extern int nc100_tty_close(uint8_t minor);
extern void nc100_tty_init(void);

extern uint8_t keymap[10];
#define KEY_ROWS	10
#define KEY_COLS	8
extern uint8_t keyboard[10][8];
extern uint8_t shiftkeyboard[10][8];

#endif
