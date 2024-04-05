#ifndef RAWUART_H
#define RAWUART_H

extern void uart1_init(void);
extern void uart1_putc(uint8_t c);
extern int uart1_ready(void);
extern int uart1_getc(void);

#endif