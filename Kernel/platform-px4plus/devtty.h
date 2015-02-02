#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__
void tty_putc(uint8_t minor, unsigned char c);
ttyready_t tty_writeready(uint8_t minor);
void tty_pollirq(void);
void tty_setup(uint8_t minor);
#endif
