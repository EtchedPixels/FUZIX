#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

extern void kbd_interrupt(void);
extern void tty_interrupt(void);
extern int mtxtty_open(uint_fast8_t minor, uint16_t flag);
extern int mtxtty_close(uint_fast8_t);
extern int mtx_vt_ioctl(uint_fast8_t minor, uarg_t request, char *data);

extern int probe_prop(void);

extern signed char vt_twidth[6];
extern signed char vt_tright[6];
extern uint8_t outputtty;

#define KEY_ROWS	8
#define KEY_COLS	16
extern uint16_t keymap[8];
extern uint8_t keyboard[8][10];
extern uint8_t shiftkeyboard[8][10];

extern uint8_t has6845;

#endif
