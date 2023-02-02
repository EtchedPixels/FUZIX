/* devtty.h		includes for 'devtty.c'		*/
#ifndef	__DEVTTY_H
#define	__DEVTTY_H
#include <kernel.h>
#include <config.h>
#include <tty.h>

void tty_putc(uint_fast8_t minor, uint_fast8_t c);

ttyready_t tty_writeready(uint_fast8_t minor);

void tty_sleeping(uint_fast8_t minor);

int tty_carrier(uint_fast8_t minor);

void tty_setup(uint_fast8_t minor, uint_fast8_t flags);

extern void tty_data_consumed(uint_fast8_t minor);

void tty1_uart_interrupt(void);		/* DO,D1,A0,A1 saved before entry */


void kputchar(uint_fast8_t c);



#endif	/* __DEVTTY2_H	*/	
