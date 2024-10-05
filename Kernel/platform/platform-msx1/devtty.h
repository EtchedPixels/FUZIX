#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

#define KEY_COLS	11
#define KEY_ROWS	8
extern uint8_t keymap[11];
extern uint8_t keyboard[11][8];
extern uint8_t shiftkeyboard[11][8];

extern void kbd_interrupt(void);
extern int vdptty_ioctl(uint8_t minor, uarg_t arg, char *ptr);
extern int vdptty_close(uint8_t minor);

extern int8_t vt_twidth;
extern int8_t vt_tright;

#endif
