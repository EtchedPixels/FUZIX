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

static char tbuf1[TTYSIZ];
static char tbuf2[TTYSIZ];

uint8_t vtattr_cap = VTA_INVERSE|VTA_FLASH|VTA_UNDERLINE;
extern uint8_t curattr;

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS
};

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
};

/* tty1 is the screen, tty2 is a 16x50 clone */

__sfr __banked __at 0xF8EF uart_rx;
__sfr __banked __at 0xF8EF uart_tx;
__sfr __banked __at 0xF8EF uart_ls;
__sfr __banked __at 0xF9EF uart_ier;
__sfr __banked __at 0xF9EF uart_ms;
__sfr __banked __at 0xFAEF uart_fcr;
__sfr __banked __at 0xFBEF uart_lcr;
__sfr __banked __at 0xFCEF uart_mcr;
__sfr __banked __at 0xFDEF uart_lsr;
__sfr __banked __at 0xFEEF uart_msr;
__sfr __banked __at 0xFFEF uart_scr;


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
	if (uart_lsr & 0x20)
		return TTY_READY_NOW;
	return TTY_READY_SOON;
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
	uint8_t msr;

	while (uart_lsr & 0x01)
		tty_inproc(2, uart_rx);
	msr = uart_msr;
	/* DCD changed - tell the kernel so it can hangup or open ports */
	if (msr & 0x08) {
		if (msr & 0x80)
			tty_carrier_raise(2);
		else
			tty_carrier_drop(2);
	}
	/* TODO: CTS/RTS */
}

int tty_carrier(uint8_t minor)
{
        if (minor == 2)
		return uart_msr & 0x80;
	return 1;
}

/*
 *	16x50 conversion betwen a Bxxxx speed rate (see tty.h) and the values
 *	to stuff into the chip.
 *
 *	The equation for this board is
 *	115200 /((DLM*256) + DLL)
 *
 *	Two extensions exist which we ignore
 *	DLM bit 7 selects native clocking ((11059200)/16/baud)
 *	DLM 0 DLL 0 = 256Kbit
 */
static uint16_t clocks[] = {
	12,		/* Not a real rate */
	2304,
	1536,
	1047,
	857,
	768,
	384,
	192,
	96,
	48,
	24,
	12,
	6,
	3,
	2,
	1
};

/*
 *	This function is called whenever the terminal interface is opened
 *	or the settings changed. It is responsible for making the requested
 *	changes to the port if possible. Strictly speaking it should write
 *	back anything that cannot be implemented to the state it selected.
 *
 *	That needs tidying up in many platforms and we also need a proper way
 *	to say 'this port is fixed config' before making it so.
 */
void tty_setup(uint8_t minor, uint8_t flags)
{
	uint8_t d;
	uint16_t w;

	if (minor == 1)
		return;

	/* 16x50. Can actually be configured */
	d = 0x80;	/* DLAB (so we can write the speed) */
	d |= (ttydata[2].termios.c_cflag & CSIZE) >> 4;
	if(ttydata[2].termios.c_cflag & CSTOPB)
		d |= 0x04;
	if (ttydata[2].termios.c_cflag & PARENB)
		d |= 0x08;
	if (!(ttydata[2].termios.c_cflag & PARODD))
		d |= 0x10;
	uart_lcr = d;
	w = clocks[ttydata[2].termios.c_cflag & CBAUD];
	uart_ls = w;
	uart_ms = w >> 8;
	uart_lcr = d & 0x7F;
	/* FIXME: CTS/RTS support */
	d = 0x03;	/* DTR RTS */
	uart_mcr = d;
	uart_ier = 0x0D;	/* We don't use tx ints */
}

void tty_sleeping(uint8_t minor)
{
	used(minor);
}

void tty_data_consumed(uint8_t minor)
{
	used(minor);
}

/* This gfx stuff needs a major revisit for all the modes */

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
