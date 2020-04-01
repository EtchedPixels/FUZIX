#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <keycode.h>
#include <vt.h>
#include <tty.h>
#include <graphics.h>
#include <input.h>
#include <devinput.h>

__sfr __banked __at 0x133B uart_status;		/* Read */
__sfr __banked __at 0x133B uart_tx;		/* Write */
__sfr __banked __at 0x143B uart_rx;		/* Read */
__sfr __banked __at 0x143B uart_baud;		/* Write */

static char tbuf1[TTYSIZ];
static char tbuf2[TTYSIZ];

static uint8_t sleeping;

uint8_t vtattr_cap = VTA_INVERSE|VTA_FLASH|VTA_UNDERLINE;
extern uint8_t curattr;

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
	_CSYS|CBAUD
};

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
};

/* tty1 is the screen, tty2 is the simple UART on the FPGA */

/* Output for the system console (kprintf etc) */
void kputchar(char c)
{
	if (c == '\n')
		tty_putc(0, '\r');
	tty_putc(0, c);
}

ttyready_t tty_writeready(uint8_t minor)
{
	if (minor == 1)
		return TTY_READY_NOW;
	if (uart_status & 0x02)
		return TTY_READY_SOON;
	return TTY_READY_NOW;
}

void tty_putc(uint8_t minor, unsigned char c)
{
	if (minor == 1)
		vtoutput(&c, 1);
	else
		uart_tx = c;
}

void tty_polluart(void)
{
	while (!(uart_status & 4))
		tty_inproc(2, uart_rx);
	if ((sleeping & 4) && (uart_status & 2)) {
		sleeping &= ~4;
		tty_outproc(2);
	}
}

int tty_carrier(uint8_t minor)
{
	used(minor);
	return 1;
}

/* Both ports are fixed configuration but the serial
   can switch baud rate */
void tty_setup(uint8_t minor, uint8_t flags)
{
	uint8_t baud = ttydata[2].termios.c_cflag & CBAUD;
	if (minor == 1)
		return;
	if (baud < B2400) {
		baud = 0;
		ttydata[2].termios.c_cflag &= ~CBAUD;
		ttydata[2].termios.c_cflag |= B2400;
	} else
		baud -= B2400;
	/* The hardware baud rates line up with our definitions from 2400 up
	   but in reverse order */
	uart_baud = 6 - baud;
}

void tty_sleeping(uint8_t minor)
{
	sleeping |= (1 << minor);
}

void tty_data_consumed(uint8_t minor)
{
	used(minor);
}

/* This is used by the vt asm code, but needs to live in the kernel */
uint16_t cursorpos;

/* For now we only support 64 char mode - we should add the mode setting
   logic and other modes FIXME */
static struct display specdisplay = {
	0,
	512, 192,
	512, 192,
	0xFF, 0xFF,
	FMT_TIMEX64,
	HW_UNACCEL,
	GFX_VBLANK|GFX_MAPPABLE|GFX_TEXT,
	0
};

/*
 *	Graphics ioctls. Not yet updated to reflect TBBlue at all
 */
int gfx_ioctl(uint8_t minor, uarg_t arg, char *ptr)
{
	if (minor != 1 || arg >> 8 != 0x03)
		return vt_ioctl(minor, arg, ptr);
	switch(arg) {
	case GFXIOC_GETINFO:
		return uput(&specdisplay, ptr, sizeof(struct display));
	case GFXIOC_WAITVB:
		/* Our system clock is vblank */
		timer_wait++;
		psleep(&timer_interrupt);
		timer_wait--;
		chksigs();
		if (udata.u_cursig) {
			udata.u_error = EINTR;
			return -1;
		}
		return 0;
	}
	return -1;
}

void vtattr_notify(void)
{
	/* Attribute byte fixups: not hard as the colours map directly
	   to the spectrum ones */
	if (vtattr & VTA_INVERSE)
		curattr =  ((vtink & 7) << 3) | (vtpaper & 7);
	else
		curattr = (vtink & 7) | ((vtpaper & 7) << 3);
	if (vtattr & VTA_FLASH)
		curattr |= 0x80;
	/* How to map the bright bit - we go by either */
	if ((vtink | vtpaper) & 0x10)
		curattr |= 0x40;
}
