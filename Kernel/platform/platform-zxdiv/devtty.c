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

uint8_t vtattr_cap = VTA_INVERSE|VTA_FLASH|VTA_UNDERLINE;
uint8_t vtborder;
uint8_t curattr = 7;

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS
};


struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
};

/* tty1 is the screen */

/* Output for the system console (kprintf etc) */
void kputchar(char c)
{
	if (c == '\n')
		tty_putc(0, '\r');
	tty_putc(0, c);
}

/* Both console and debug port are always ready */
ttyready_t tty_writeready(uint8_t minor)
{
	minor;
	return TTY_READY_NOW;
}

void tty_putc(uint8_t minor, unsigned char c)
{
	minor;
	vtoutput(&c, 1);
}

int tty_carrier(uint8_t minor)
{
	minor;
	return 1;
}

void tty_setup(uint8_t minor, uint8_t flags)
{
	minor;
}

void tty_sleeping(uint8_t minor)
{
	minor;
}

void tty_data_consumed(uint8_t minor)
{
}


/* This is used by the vt asm code, but needs to live in the kernel */
uint16_t cursorpos;

static struct display specdisplay = {
	0,
	256, 192,
	256, 192,
	0xFF, 0xFF,
	FMT_SPECTRUM,
	HW_UNACCEL,
	GFX_VBLANK|GFX_MAPPABLE|GFX_TEXT,
	0
};

static struct videomap specmap = {
	0,
	0,
	0x4000,
	6912,
	0,
	0,
	0,
	MAP_FBMEM|MAP_FBMEM_SIMPLE
};

/*
 *	Graphics ioctls. Very minimal for this platform. It's a single fixed
 *	mode with direct memory mapping.
 */
__sfr __at 0xFE border;

int gfx_ioctl(uint8_t minor, uarg_t arg, char *ptr)
{
	uint8_t n;

	if (minor == 1) {
		switch(arg) {
		case GFXIOC_GETINFO:
			return uput(&specdisplay, ptr, sizeof(struct display));
		case GFXIOC_MAP:
			return uput(&specmap, ptr, sizeof(struct videomap));
		case GFXIOC_UNMAP:
			return 0;
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
		case VTBORDER:
			n = ugetc(ptr);
			vtborder &= 0xF8;
			vtborder |= (n & 0x07);
			border = vtborder;
			return 0;
		}
	}
	return vt_ioctl(minor, arg, ptr);
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
