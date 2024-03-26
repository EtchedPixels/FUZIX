#ifndef RAWUART_H
#define RAWUART_H

extern void uart1_init();
extern void uart1_putc(uint8_t c);
extern int uart1_ready();
extern int uart1_getc();

#endif