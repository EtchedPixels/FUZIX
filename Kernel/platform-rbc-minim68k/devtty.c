#include <kernel.h>
#include <kdata.h>
#include <stdbool.h>
#include <tty.h>
#include "devtty.h"
#include "uart.h"

/*
	Excerpts from .../Kernel/../TTY.txt
*/

/*****************************************************************************
Kernel Interface
----------------

**int tty_open(uint_fast8_t minor, uint16_t flag)**

This method handles the opening of a tty device. On success it will call
into tty_setup as provided by the platform and then wait for the carrier
if appropriate. tty_setup can be used to handle things like power
management of level shifters.

**int tty_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)**

This method implements the normal read behaviour for a terminal device
and is generally called directly as the device driver method. It will
call into the platform code in order to implement the low level
behaviour.

**int tty_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)**

This method implements the normal write behaviour for a terminal device
and is generally called directly as the device driver method. It will
call into the platform code in order to implement the low level
behaviour.

**int tty_ioctl(uint_fast8_t minor, uarg_t request, char \*data)**

Implements the standard tty and line discipline ioctls. This can be
wrapped by platform specific code to provide additional ioctl
interfaces, or by other higher layers (see Virtual Terminal) for the
same purpose.

**tty_close(uint_fast8_t minor)**

Called on the last close of the open tty file handle. This can be
wrapped by drivers that need to reverse some action taken in tty_setup.
*/

/*****************************************************************************
Driver Helper Functions
-----------------------

**int tty_inproc(uint_fast8_t minor, unsigned char c)**

Queue a character to the tty interface indicated. This method is
interrupt safe. This may call back into the platform tty output code in
order to implement echoing of characters.

**void tty_outproc(uint_fast8_t minor)**

When implementing asynchronous output indicates that more data may be
queued to the hardware interface. May be called from an interrupt
handler. Any output restarted will restart fom a non interrupt context
after the IRQ completes.

**tty_putc_wait(uint_fast8_t minor, unsigned char c)**

Write a character to the output device, waiting if necessary. Not
interrupt safe.

**tty_carrier_drop(uint_fast8_t minor)**

Indicate that the carrier line on this port has been dropped. Can be
called from an interrupt.

**tty_carrier_raise(uint_fast8_t minor)**

Indicate that the carrier line on this port has been raised. Can be
called from an interrupt.
*/

/*****************************************************************************
Defines Provided By The Platform
------------------------------------

**NUM_DEV_TTY:** The number of actual tty device ports (one based not zero based)

**TTYDEV:** The device used for the initial init process and for boot prompts

Structures Provided By The Platform
-----------------------------------

**unsigned char [TTYSIZ]**

One buffer per tty instance.

**struct s_queue[NUM_DEV_TTY+1]**

One queue per tty plus queue 0 which is not used. The structure is set
up as

{ buffer, buffer, buffer, TTYSIZ, 0, TTYSIZ / 2 }
*/



typedef uint8_t tty_buf_t[TTYSIZ];


static tty_buf_t buffer[NUM_DEV_TTY];

struct s_queue	ttyinq[NUM_DEV_TTY+1] = {
	{NULL, NULL, NULL, TTYSIZ, 0, TTYSIZ/2 },
/* minor==1  uses buffer[0]; i.e., buffer[minor-1]	*/
	{buffer[0], buffer[0], buffer[0], TTYSIZ, 0, TTYSIZ/2},   /* tty1 */
#if NUM_DEV_TTY > 1
	{buffer[1], buffer[1], buffer[1], TTYSIZ, 0, TTYSIZ/2},   /* tty2 */
#endif
#if NUM_DEV_TTY > 2
	{buffer[2], buffer[2], buffer[2], TTYSIZ, 0, TTYSIZ/2},   /* tty3 */
#endif
#if NUM_DEV_TTY > 3
	{buffer[3], buffer[3], buffer[3], TTYSIZ, 0, TTYSIZ/2},   /* tty4 */
#endif
#if NUM_DEV_TTY > 4
	{buffer[4], buffer[4], buffer[4], TTYSIZ, 0, TTYSIZ/2},   /* tty5 */
#endif
#if NUM_DEV_TTY > 5
	{buffer[5], buffer[5], buffer[5], TTYSIZ, 0, TTYSIZ/2},   /* tty6 */
#endif
#if NUM_DEV_TTY > 6
	{buffer[6], buffer[6], buffer[6], TTYSIZ, 0, TTYSIZ/2},   /* tty7 */
#endif
#if NUM_DEV_TTY > 7
	{buffer[7], buffer[7], buffer[7], TTYSIZ, 0, TTYSIZ/2},   /* tty8 */
#endif
#if NUM_DEV_TTY > 8
# error "Too many TTY devices (NUM_DEV_TTY>8)"
#endif
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
#if NUM_DEV_TTY > 1
#endif
#if NUM_DEV_TTY > 2
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
#endif
#if NUM_DEV_TTY > 3
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
#endif
#if NUM_DEV_TTY > 4
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
#endif
#if NUM_DEV_TTY > 5
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
#endif
#if NUM_DEV_TTY > 6
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
#endif
#if NUM_DEV_TTY > 7
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
#endif
};




/*****************************************************************************
Methods Provided By The Platform
------------------------------------

**void tty_putc(uint_fast8_t minor, unsigned char c)**

Write a character to the tty device. Non-blocking. If the device is busy
drop the byte. When handling echo on a particularly primitive port it
may be advantageous to implement a one byte buffer in the driver code.

**ttyready_t tty_writeready(uint_fast8_t minor)**

Report whether the device can accept a byte of data. This should return
either TTY_READY_NOW (you may write), TTY_READY_SOON (polling may be
used) or TTY_READY_LATER (there is no point polling). The use of
TTY_READY_SOON allows slow platforms to avoid the continuous overhead of
terminal interrupts, or deferring writes until the next timer tick. The
kernel will poll until the process would naturally be pre-empted. On
fast devices it may be worth always returning TTY_READY_LATER. When the
port is blocked due to a long standing condition such as flow control
TTY_READY_LATER should be returned.

**void tty_sleeping(uint_fast8_t minor)**

This method is called just before the kernel exits polling of a tty
port. This allows the driver to selectively enable transmit complete
interrupts in order to reduce CPU loading. For other platforms this may
be a null function.

**int tty_carrier(uint_fast8_t minor)**

Report the carrier status of the port. If the port has no carrier
control then simply return 1 (carrier present).

void tty_setup(uint_fast8_t minor)

Perform any hardware set up necessary to open this port. If none is
required then this can be a blank function.

**void kputchar(char c)**

Writes a character from the kernel to a suitable console or debug port.
This is usually implemented in terms of tty_putc. As it may be called
from an interrupt it cannot use tty_putc_wait(). On platforms with queued
interrupt driven output this routine should ideally not return until the
character is visible to the user.
*/

#ifdef	CONFIG_16x50

#define CTS (1<<B_CTS)
#define DCD (1<<B_DCD)
#define DSR (1<<B_DSR)
#define THRE (1<<B_THRE)


/*
 *	16x50 conversion betwen a Bxxxx speed rate (see tty.h) and the values
 *	to stuff into the chip.
 */
#define OSCILLATOR 1843200	/* 1.8432Mhz */
#define	UCLOCK (OSCILLATOR/16)
static uint16_t clocks[] = {
	0,		/* Use the BIOS setup rate */
	UCLOCK/50,	/* 2304 */
	UCLOCK/75,	/* 1536 */
	UCLOCK/110,	/* 1047 */
	858,		/* nearly 134.246 bps (actually 134.2657)*/
	UCLOCK/150,	/* 768 */
	UCLOCK/300,	/* 384 */
	UCLOCK/600,	/* 192 */
	UCLOCK/1200,	/* 96 */
	UCLOCK/2400,	/* 48 */
	UCLOCK/4800,	/* 24 */
	UCLOCK/9600,	/* 12 */
	UCLOCK/19200,	/* 6 */
	UCLOCK/38400,	/* 3 */
	UCLOCK/57600,	/* 2 */
	UCLOCK/115200	/* 1 */
};




struct tty_connect_s {
	uint8_t ioport;			/* used as IOMEM(port) */
	U16x50_t flavor;		/* 8250A, 16550A, &c.	     */
	uint8_t interrupt;		/* interrupt channel */
} tty_connect[NUM_DEV_TTY+1] = {
	{0xFF, Unone, 0xFF},		/* no minor 0 */
#ifdef INT_TTY1
	{0x48, U16550A, INT_TTY1},	/* console better be at 0x48, chip flavor
					   will likely be updated */
#else
	{0x48, U16550A, 0xFF},
#endif
#if 0
/*** device search will have to fill in the rest of this array on the fly ***/
	{0xFF, Unone, 0xFF},		/* no minor 2 */
	{0xFF, Unone, 0xFF},		/* no minor 3 */
	{0xFF, Unone, 0xFF},		/* no minor 4 */
	{0xFF, Unone, 0xFF},		/* no minor 5 */
	{0xFF, Unone, 0xFF},		/* no minor 6 */
	{0xFF, Unone, 0xFF},		/* no minor 7 */
	{0xFF, Unone, 0xFF},		/* no minor 8 */
#endif	
};

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	volatile uint8_t *port = IOMEM(tty_connect[minor].ioport);

	while ( (port[UART_LSR] & THRE) == 0 ) /* SPIN */ ;

	/* Transmitter Holding Register is now empty */
	port[UART_THR] = c;
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
	volatile uint8_t *port = IOMEM(tty_connect[minor].ioport);

	uint8_t msr = port[UART_MSR];

	if (ttydata[minor].termios.c_cflag & CRTSCTS) {
		if ((msr & CTS) == 0 )  /* The modem says don't send at the moment */
			return TTY_READY_LATER;
	}
	if (port[UART_LSR] & THRE) /* THR is empty */
			return TTY_READY_NOW;
	return TTY_READY_SOON;		/* No serious hold up to sending */
}

void tty_sleeping(uint_fast8_t minor)
{
}

int tty_carrier(uint_fast8_t minor)
{
	volatile uint8_t *port = IOMEM(tty_connect[minor].ioport);
	uint8_t msr = port[UART_MSR];
	if ((msr & DCD) == DCD) 
		return 1;	/* carrier present */
	return 0;
}

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
	volatile uint8_t *port;
	tcflag_t mask;
	uint8_t iport, lcr;
	uint8_t uchar;	/* character out composed here */
	uint16_t baud;
	
	if ( minor > NUM_DEV_TTY 
		|| (iport = tty_connect[minor].ioport) == 0xFF) return;
	port = IOMEM(iport);
	mask = termios_mask[minor];

	port[UART_MCR] = 0x3;		/* DTR & RTS */
	uchar = 0;
	if (tty_connect[minor].interrupt < 16) {   /* 0xFF used to disable */
		uchar = (1<<B_ERBI);		
	}
	port[UART_IER] = uchar;
#if (CSIZE|CSTOPB) == 0x70	
	uchar = (mask & (CSIZE|CSTOPB))>>4;
#else
# error CSIZE CSTOPB alignment problem
#endif
	/* set DLAB and do baud rate and FIFO setup */
	lcr = port[UART_LCR];
	port[UART_LCR] = lcr | (1<<B_DLAB);
	baud = clocks[mask & CBAUD];
	/* baud is the divisor needed */
	if (baud != 0) {	/* if == 0, then use BIOS setup rate */
		port[UART_DLL] = (uint8_t)baud;		/* low order */
		port[UART_DLH] = (uint8_t)(baud >> 8);	/* high order */
	}
	if (tty_connect[minor].flavor >= U16550A) {	/* has FIFO */
		port[UART_FCR] = 0x87;	/* reset FIFO's, enable 16 char, threshold 8 */
	}
	else if (tty_connect[minor].flavor == U16550) {
		port[UART_FCR] = 0;			/* disable broken FIFO */
	}  /* lower UARTs don't even have a FIFO */
	port[UART_LCR] = lcr;		/* reset DLAB */
}

/* void tty_putc_wait(uint_fast8_t minor, uint_fast8_t c) */
void tty1_uart_interrupt(void)		/* DO,D1,A0,A1 saved before entry */
{
	volatile uint8_t *port = IOMEM(tty_connect[1].ioport);
	uint8_t lsr, data;
	uint_fast8_t bad = 0;
#ifdef INT_TTY1
	uint8_t iir, msr;

		/* IRR bits
		 * 3 2 1 0
		 * -------
		 * x x x 1     no interrupt pending
		 * 0 1 1 0  6  LSR changed -- read the LSR
		 * 0 1 0 0  4  receive FIFO >= threshold
		 * 1 1 0 0  C  received data sat in FIFO for a while
		 * 0 0 1 0  2  transmit holding register empty
		 * 0 0 0 0  0  MSR changed -- read the MSR
		 */
	
	while (((iir = port[UART_IIR]) & 1) == 0) {
		lsr = port[UART_LSR];
		switch (iir & 0xE) {
		   case 6: /* LSR changed */
			if (lsr & ((1<<B_OE)|(1<<B_PE)|(1<<B_FE)|(1<<B_BRE)))
				bad = 1;
			break;
		   case 4:
	           case 12:  /* data available */
#endif
	           	while ((lsr = port[UART_LSR]) & (1<<B_DR)) {
				data = port[UART_RBR];
// Need to add to core 		if (bad) {
//					tty_inproc_bad(1, data);
//					bad = 0;
//				}
//				else
					tty_inproc(1, data);
			}
#ifdef INT_TTY1
			break;
		   case 2:   /* THR Empty */
			break;
		   case 0:   /* MSR changed */
			msr = port[UART_MSR];
			if (msr & 0x08) {
				if (msr & 0x80)
					tty_carrier_raise(1);
				else
					tty_carrier_drop(1);
			}
			if ((msr & 0x11) == 0x11)
				tty_outproc(1);
			
			break;
		   default:  /* do nothing */
			break; 	
		}  /* switch */
		
	}  /* while */
#endif
}

/* TODO: CTS/RTS handling */

void tty_data_consumed(uint_fast8_t minor)
{
}

void kputchar(uint_fast8_t c)
{
	if (c == '\n') tty_putc(1, '\r');
	tty_putc(1, c);
}
#endif
