#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

#define UART_NAME 0x0F
#define UART_CAP_FIFO 0x80		/* UART has functioning FIFO */
#define UART_CAP_AFE 0x40		/* UART support autoflow control */
#define UART_8250 1
#define UART_16450 2
#define UART_16550 3
#define UART_16550A 4

#define UART_CLOCK 1843200UL
void tty_putc(uint8_t minor, unsigned char c);
void tty_pollirq_uart0(void);
void uart0_init(void);

#ifdef CONFIG_PPP
void tty_poll_ppp(void);
#endif
#endif
