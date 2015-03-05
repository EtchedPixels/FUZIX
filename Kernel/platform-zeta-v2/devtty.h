#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__
void tty_putc(uint8_t minor, unsigned char c);
void tty_interrupt(void);

#ifdef CONFIG_PPP
void tty_poll_ppp(void);
#endif
#endif
