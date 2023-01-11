#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

void handle_keys(void);

#define KEY_ROWS	8
#define KEY_COLS	8
extern uint8_t keymap[8];
extern uint8_t keyboard[8][8];
extern uint8_t shiftkeyboard[8][8];

#endif
