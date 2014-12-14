#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

extern void kbd_interrupt(void);

extern signed char vt_twidth[2];
extern signed char vt_tright[2];
extern uint8_t curtty;

#endif
