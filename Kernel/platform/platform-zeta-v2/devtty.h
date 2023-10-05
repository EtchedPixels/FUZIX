#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

#define UART_NAME 0x0F
#define UART_CAP_FIFO 0x80		/* UART has a functioning FIFO */
#define UART_CAP_AFE 0x40		/* UART supports auto-flow control */
#define UART_8250 1			/* original INS8250 or INS8250-B */ 
#define UART_16450 2			/* INS8250A, 16450 and clones */
#define UART_16550 3			/* original 16550 with broken FIFO */
#define UART_16550A 4			/* 16550A and clones */

#define UART_CLOCK 1843200UL
void tty_putc(uint8_t minor, unsigned char c);
void tty_pollirq_uart0(void);
void uart0_init(void);

#endif
