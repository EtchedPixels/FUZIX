#ifndef _DEVTTY_H
#define _DEVTTY_H

extern void tty_interrupt(void);
extern void kbd_interrupt(void);
extern void keyscan(void);
extern void mousescan(void);
extern uint8_t mouse_probe(void);	/* 0 means yes */
extern int16_t mouse12(uint8_t *) __z88dk_fastcall;

extern uint8_t mouse_present;

extern void queue_input(uint8_t);

#define KEY_ROWS	9
#define KEY_COLS	8

extern uint8_t keymap[9];
extern uint8_t keyboard[9][8];
extern uint8_t shiftkeyboard[9][8];

extern int gfx_ioctl(uint_fast8_t minor, uarg_t arg, char *ptr);

#endif
