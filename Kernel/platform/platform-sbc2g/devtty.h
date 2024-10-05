#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

/* SIO 2 ports */

#define SIO0_BASE 0x00
#define SIOA_D	(SIO0_BASE + 0)
#define SIOB_D	(SIO0_BASE + 1)
#define SIOA_C	(SIO0_BASE + 2)
#define SIOB_C	(SIO0_BASE + 3)

extern void sio2_otir(uint8_t port);

void tty_putc(uint_fast8_t minor, uint_fast8_t c);
void tty_poll(void);

#endif
