#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

void tty_putc(uint_fast8_t minor, uint_fast8_t c);
void tty_pollirq(void);

#endif
