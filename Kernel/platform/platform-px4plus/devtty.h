#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__
void tty_putc(uint8_t minor, unsigned char c);
void tty_pollirq(void);
void tty_setup(uint8_t minor);
#endif
