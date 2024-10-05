#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

#define KEY_ROWS 8
#define KEY_COLS 7
extern uint8_t keymap[8];
extern uint8_t keyboard[8][7];
extern uint8_t shiftkeyboard[8][7];

extern int gfx_ioctl(uint_fast8_t minor, uarg_t arg, char *ptr);

#endif
