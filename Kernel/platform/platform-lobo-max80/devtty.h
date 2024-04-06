#ifndef _DEVTTY_H
#define _DEVTTY_H

extern void tty_interrupt(void);
extern void kbd_interrupt(void);
extern void vtbuf_init(void);
extern int lobotty_ioctl(uint_fast8_t minor, uarg_t arg, char *ptr);

#define KEY_ROWS	8
#define KEY_COLS	8
extern uint8_t keymap[8];
extern uint8_t keyboard[8][8];
extern uint8_t shiftkeyboard[8][8];

extern uint8_t sio_r[];
extern void sio2_out(uint_fast8_t minor);

#endif
