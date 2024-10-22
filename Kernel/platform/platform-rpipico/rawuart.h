#ifndef RAWUART_H
#define RAWUART_H

#include <tty.h>

extern void rawuart_early_init(void);
extern void rawuart_putc(uint8_t devn, uint8_t c);
extern ttyready_t rawuart_ready(uint8_t devn);
extern void rawuart_sleeping(uint8_t devn);
extern int rawuart_getc(uint8_t devn);
extern void rawuart_setup(uint_fast8_t minor, uint_fast8_t devn, uint_fast8_t flags);

#endif