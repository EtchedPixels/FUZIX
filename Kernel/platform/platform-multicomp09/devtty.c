#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <device.h>
#include <tty.h>
#include <devdw.h>
#include <ttydw.h>

#undef  DEBUG			/* UNdefine to delete debug code sequences */
#undef  MC09_VIRTUAL_IN

/* Hardware Registers */
#define TIMER  (0xffdd)
#define timer_reg  *((volatile uint8_t *)TIMER)


/* Multicomp has 3 serial ports. Each is a cut-down 6850, with fixed BAUD rate and word size.
   Port 0 is, by default, a virtual UART interface to a VGA output and PS/2 keyboard
   Port 1 is, by default, a serial port
   Port 0 and Port 1 mappings can be swapped through a jumper on the PCB.
   Port 2 is a serial port.

   Port 0 is used for tty1, Port 1 for tty2. Port2 is dedicated to DriveWire.
*/
static uint8_t *uart[] = {
	0,      0,                               /* Unused */
	(uint8_t *)0xFFD1, (uint8_t *)0xFFD0,    /* Virtual UART Data, Status port0, tty1 */
	(uint8_t *)0xFFD3, (uint8_t *)0xFFD2,    /*         UART Data, Status port1, tty2 */
	(uint8_t *)0xFFD5, (uint8_t *)0xFFD4,    /*         UART Data, Status port2, dw   */
};

#ifdef MC09_VIRTUAL_IN
/* Feed characters in to the console - for use in emulation because
   my emulator currently does not support non-blocking input and it's
   more fun making progress here than fixing the emulator.
*/
static int icount = 0;
static int imatch = 100;
/* X represents a pause at end-of-line*/
/* Without pauses, the input streams ahead of the output */
/*static uint8_t input[] = "ls -al\nXpwd\nXps\nX\x04root\nXtime ls\nX\nfforth /usr/src/fforth/fforth_tests.fth\nX"; */
static uint8_t input[] = "ls -al\nXpwd\nX";
static int ccount = 0;
#endif


static uint8_t tbuf1[TTYSIZ];   /* virtual serial port 0: console */
static uint8_t tbuf2[TTYSIZ];   /*         serial port 1: UART */
static uint8_t tbuf3[TTYSIZ];   /* drivewire VSER 0 */
static uint8_t tbuf4[TTYSIZ];   /* drivewire VSER 1 */
static uint8_t tbuf5[TTYSIZ];   /* drivewire VSER 2 */
static uint8_t tbuf6[TTYSIZ];   /* drivewire VSER 3 */
static uint8_t tbuf7[TTYSIZ];   /* drivewire VWIN 0 */
static uint8_t tbuf8[TTYSIZ];   /* drivewire VWIN 1 */
static uint8_t tbuf9[TTYSIZ];   /* drivewire VWIN 2 */
static uint8_t tbufa[TTYSIZ];   /* drivewire VWIN 3 */


struct s_queue ttyinq[NUM_DEV_TTY + 1] = {
	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	/* Virtual UART/Real UART Consoles */
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
	/* Drivewire Virtual Serial Ports */
	{tbuf3, tbuf3, tbuf3, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf4, tbuf4, tbuf4, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf5, tbuf5, tbuf5, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf6, tbuf6, tbuf6, TTYSIZ, 0, TTYSIZ / 2},
	/* Drivewire Virtual Window Ports */
	{tbuf7, tbuf7, tbuf7, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf8, tbuf8, tbuf8, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf9, tbuf9, tbuf9, TTYSIZ, 0, TTYSIZ / 2},
	{tbufa, tbufa, tbufa, TTYSIZ, 0, TTYSIZ / 2},
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	/* Virtual UART */
	_CSYS,
	_CSYS,
	/* Drivewire */
	_CSYS,
	_CSYS,
	_CSYS,
	_CSYS,
	/* Virtual Window */
	_CSYS,
	_CSYS,
	_CSYS,
	_CSYS
};


/* A wrapper for tty_close that closes the DW port properly */
int my_tty_close(uint_fast8_t minor)
{
	if (minor > 2 && ttydata[minor].users == 1)
		dw_vclose(minor);
	return (tty_close(minor));
}


/* Output for the system console (kprintf etc) */
void kputchar(uint_fast8_t c)
{
	uint8_t minor = minor(TTYDEV);

	while ((*(uart[minor*2 + 1]) & 2) != 2) {
		/* UART is busy */
	}

	/* convert from CR to CRLF */
	if (c == '\n') {
		tty_putc(minor, '\r');
		while ((*(uart[minor*2 + 1]) & 2) != 2) {
			/* UART is busy */
		}
	}

	tty_putc(minor, c);
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
	uint8_t c;
        if ((minor < 1) || (minor > 2)) {
		return TTY_READY_NOW;
        }
	c = *(uart[minor*2 + 1]); /* 2 entries per UART, +1 to get STATUS */
	return (c & 2) ? TTY_READY_NOW : TTY_READY_SOON; /* TX DATA empty */
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	if ((minor > 0) && (minor < 3)) {
		*(uart[minor*2]) = c; /* UART Data */
	}
	if (minor > 2 ) {
		dw_putc(minor, c);
	}
}

void tty_sleeping(uint_fast8_t minor)
{
	used(minor);
}


void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
	if (minor > 2) {
		dw_vopen(minor);
		return;
	}
}


int tty_carrier(uint_fast8_t minor)
{
	if( minor > 2 ) return dw_carrier( minor );
	return 1;
}

void tty_interrupt(void)
{

}

void tty_data_consumed(uint_fast8_t minor)
{
}


void plt_interrupt(void)
{
	uint8_t c;
	/* Check each UART for characters and dispatch if available
	   .. assuming I eventually get around to enabling serial Rx interrupts
	   this will just get perkier with no additional coding required
	   [NAC HACK 2016May05]  enable serial interrupts!!
	*/

#ifdef MC09_VIRTUAL_IN
	icount++;
	if (icount == imatch) {
		imatch += 200;
		if (input[ccount] != 0) {
			while (input[ccount] != 'X') {
				tty_inproc(minor(TTYDEV), input[ccount++]);
			}
			ccount++;
		}
	}
#else
	c = *(uart[1*2 + 1]);
	if (c & 0x01) { tty_inproc(1, *(uart[1*2])); }
	/*	c = *(uart[2*2 + 1]);
	if (c & 0x01) { tty_inproc(2, *(uart[2*2])); } */
#endif

	c = timer_reg;
	if (c & 0x80) {
		timer_reg = c;       /* service the hardware */
		timer_interrupt();   /* tell the OS it happened */
	}

	dw_vpoll();
}

void plt_reinterrupt(void)
{
	panic("reint");
}


/* Initial Setup stuff down here. */

__attribute__((section(".discard")))
void devtty_init()
{
	/* Reset each UART by write to STATUS register */
	*uart[3] = 3;
	*uart[5] = 3;
	*uart[7] = 3;
}
