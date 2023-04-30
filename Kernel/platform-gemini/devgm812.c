/*
 *	GM812/821/827 80x25 intelligent video controller
 *
 *	We usually need this for Gemini but not Nascom although it could live
 * 	in either in which case we have two keyboards and consoles. We'll
 *	figure out how to support both at some point later
 *
 *	The 811 is the original, the 821 and 827 add some features.
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <vt.h>
#include <devtty.h>
#include <input.h>
#include <devinput.h>
#include <stdarg.h>

char tbuf1[TTYSIZ];
char tbuf2[TTYSIZ];

__sfr __at 0xB1 data;
__sfr __at 0xB2 status;
__sfr __at 0xB3 reset;

static uint8_t kbdstate;

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
};

/* Write to system console */
void kputchar(char c)
{
	if (c == '\n')
		tty_putc(1, '\r');
	tty_putc(1, c);
}

/*
 *	Ensure we don't mess up our keyboard scans with output. We hook
 *	tty_writeready and if need be lie to the upper layer about there
 *	being space while we stuff in our escape codes
 *
 *	States  0: idle
 *		1: waiting to send escape
 *		2: waiting to send 'k'
 *		3: waiting keyboard pending reply
 *		4: waiting to send escape
 *		5: waiting to send 'K'
 *		6: waiting keyboard character
 */
static void kbdop(void)
{
	if (kbdstate == 1 || kbdstate == 4) {
		data = '\033;
		kbdstate++;
		return;
	}
	if (kbdstate == 2) {
		data = 'k';
		kbdstate++;
		return;
	}
	if (kbdstate == 5) {
		data = 'K';
		kbdstate++;
		return;
	}
}
		

ttyready_t tty_writeready(uint8_t minor)
{
	uint8_t reg;
	reg = status;
	while (!(reg & 0x01)) {
		if (kbdstate)
			kbdop();
		if (kbdstate)
			continue;
		return TTY_READY_NOW;
	}
	return TTY_READY_SOON;
}

/* Used to track, filter and control the port */
static uint8_t state;

/* This is insufficient. The incredibly clunky polling interface via esc
   codes means we need to buffer an complete escape sequence of any valid
   form *before* we output it as it must be unbroken by a keyboard poll ! */
void tty_putc(uint8_t minor, unsigned char c)
{
	/* Screen problematic escape sequences */
	if (state == 1) {
		if (c == '?' || c == 'F' || c == 'L' || c == 'P' ||
			c == 'U' || c== 'X' || c == 'K' || c == 'k' ||
			c == 'Z')
				c = 'V';	/* render harmless */
		}
		state = 0;
	}
	
	if (c == '\033') {
		state = 1;
		return;
	}
	data = c;
}

/* Called every event */
void tty_poll(void)
{
	if (!kbdstate)
		return;
	/* Input wait states */
	if (kbdstate == 3 || kbdstate == 6) {
		if (status & 0x80)
			return;
		r = data;
		if (kbdstate == 3) {
			if (r == 0xFF) {
				kbdstate++;
				kbdop();
			} else
				kbdstate = 0;	/* No data */
			return;
		}
		/* State 6 - actual data */
		tty_inproc(1, r);
		kbdstate = 0;
	}
	/* Try and move to the next state by writing a byte */
	kbdpoll();
}

/* Called on timer events */
void kbd_poll(void)
{
	/* Kick off a regular poll */
	if (kbdstate == 0)
		kbdstate = 1;
}

void tty_setup(uint8_t minor)
{
	/* The console is directly handled. We do need to deal with the 8250
	   on a GM813 main board */
}

int tty_carrier(uint8_t minor)
{
	return 1;
}

void tty_sleeping(uint8_t minor)
{
	used(minor);
}

void tty_data_consumed(uint8_t minor)
{
	used(minor);
}

