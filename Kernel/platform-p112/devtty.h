#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__
void tty_putc(uint8_t minor, unsigned char c);
void tty_pollirq_escc(void);
void tty_pollirq_asci0(void);
void tty_pollirq_asci1(void);
#endif
