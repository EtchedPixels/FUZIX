#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

#define UART_CLOCK 1843200UL
void tty_putc(uint8_t minor, unsigned char c);
void tty_pollirq_uart0(void);

#ifdef CONFIG_PPP
void tty_poll_ppp(void);
#endif
#endif
