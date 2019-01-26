#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

void tty_putc(uint8_t minor, unsigned char c);
void tty_pollirq_sio(void);
void tty_poll_cpld(void);

extern uint8_t ser_type;

#endif
