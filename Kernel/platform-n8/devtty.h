#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__
void tty_pollirq_asci0(void);
void tty_pollirq_asci1(void);

/* For console */
extern uint8_t vt_twidth, vt_tright;
#endif
