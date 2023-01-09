#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__
void tty_pollirq_asci0(void);
void tty_pollirq_asci1(void);

/* For console */
extern uint8_t vt_twidth, vt_tright;

extern uint_fast8_t kbd_open;		/* Counts open consoles */

int n8tty_open(uint_fast8_t minor, uint16_t flag);
int n8tty_close(uint_fast8_t minor);

#endif
