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

extern uint8_t fontdata_8x8[];

static char tbuf1[TTYSIZ];

uint8_t vtattr_cap = VTA_INVERSE | VTA_FLASH | VTA_UNDERLINE;
uint8_t vtborder;
uint8_t video_mode;
uint8_t radastan;

extern uint8_t curattr;

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS
};


struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{ NULL, NULL, NULL, 0, 0, 0 },
	{ tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2 },
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
	if (video_mode == 0)
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
	GFX_VBLANK | GFX_MAPPABLE | GFX_TEXT,
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
	MAP_FBMEM | MAP_FBMEM_SIMPLE
};

/*
 *	Graphics glue
 */

__sfr __at 0xFE border;

static struct fontinfo fontinfo = {
	0, 255, 128, 255, FONT_INFO_8X8
};

static struct display vid_mode[3] = {
	{	/* Timex hi-res */
		0,
		512, 192,
		512, 192,
		255, 255,
		FMT_TIMEX64,
		HW_UNACCEL,
		GFX_MULTIMODE|GFX_TEXT|GFX_VBLANK,
		16,
		0,
		64,24
	},{	/* Timex hi-colour */
		1,
		256, 192,
		256, 192,
		255, 255,
		FMT_TIMEX64,
		HW_UNACCEL,
		GFX_MULTIMODE|GFX_VBLANK,
		16,
		0,
		32,24
	},{	/* Sinclair */
		2,
		256, 192,
		256, 192,
		255, 255,
		FMT_SPECTRUM,
		HW_UNACCEL,
		GFX_MULTIMODE|GFX_VBLANK,
		7,
		0,
		32,24
	}, {	/* Radastan (Uno only) */
		3,
		128, 96,
		256, 128,
		1, 2,
		FMT_COLOUR16,
		HW_UNACCEL,
		GFX_MULTIMODE|GFX_VBLANK|GFX_OFFSCREEN|GFX_WRAP,
		16,
		GFX_SCROLL,
		16,12
	}
};

static uint8_t modebits[] = {
	6, 2, 0
};

__sfr __at 0xFF timex;

__sfr __banked __at 0xfc3b uno_ctrl;
__sfr __banked __at 0xfd3b uno_data;

__sfr __banked __at 0xbf3b ula_ctrl;
__sfr __banked __at 0xff3b ula_data;

static void video_set(uint8_t mode)
{
	irqflags_t irq = di();

	if (mode < 3) {
		if (radastan) {
			uno_ctrl = 64;
			uno_data = 0;
		}
		portff &= 0xF8;
		portff |= modebits[mode];
		timex = portff;
		video_mode = mode;
	} else {
		/* Radastan, no offset, pad for scrolling */
		uno_ctrl = 15;
		uno_data = 0;
		uno_ctrl = 64;
		uno_data = 3;
		uno_ctrl = 65;
		uno_data = 0;
		uno_data = 0;
		uno_ctrl = 66;
		uno_data = 128;
		video_mode = mode;
	}
	irqrestore(irq);
}

int gfx_ioctl(uint8_t minor, uarg_t arg, char *ptr)
{
	uint8_t n;

	if (minor == 1) {
		switch (arg) {
		case GFXIOC_GETINFO:
			return uput(&specdisplay, ptr, sizeof(struct display));
#if 0
/* Need to add gfx binary support and other gunge */
		case GFXIOC_MAP:
			return uput(&specmap, ptr, sizeof(struct videomap));
		case GFXIOC_UNMAP:
			return 0;
#endif
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
		case GFXIOC_GETMODE:
		case GFXIOC_SETMODE: {
			n = ugetc(ptr);
			/* Mode 3 is uno specific */
			if (n > 2 + radastan)  {
				udata.u_error = EINVAL;
				return -1;
			}
			if (arg == GFXIOC_GETMODE)
				return uput(&vid_mode[n], ptr, sizeof(struct display));
			video_set(n);
			return 0;
		}
		case GFXIOC_SCROLL: {
			uint16_t xs = ugetw(ptr);
			uint16_t ys = ugetw(ptr + 2);
			/* 256 pixels but step is 2 */
			if (video_mode != 3 || xs > 128 || ys > 128) {
				udata.u_error = EINVAL;
				return -1;
			}
			/* Turn into a byte offset */
			xs += ys * 128;
			uno_ctrl = 65;
			uno_data = xs;
			uno_data = xs >> 8;
			return 0;
		}
		/* TODO: ULAplus palette */
		case VTBORDER:
			n = ugetc(ptr);
			vtborder &= 0xF8;
			vtborder |= (n & 0x07);
			border = vtborder;
			return 0;
		case VTFONTINFO:
			return uput(&fontinfo, ptr, sizeof(fontinfo));
		case VTSETFONT:
			return uget(ptr, fontdata_8x8, 2048);
		case VTGETFONT:
			return uput(fontdata_8x8, ptr, 2048);
		case VTSETUDG:
			n = ugetc(ptr);
			ptr++;
			if (n < 128) {
				udata.u_error = EINVAL;
				return -1;
			}
			return uget(fontdata_8x8 + n * 8, ptr, 8);
		}
	}
	return vt_ioctl(minor, arg, ptr);
}

void vtattr_notify(void)
{
	/* Attribute byte fixups: not hard as the colours map directly
	   to the spectrum ones */
	if (vtattr & VTA_INVERSE)
		curattr = ((vtink & 7) << 3) | (vtpaper & 7);
	else
		curattr = (vtink & 7) | ((vtpaper & 7) << 3);
	if (vtattr & VTA_FLASH)
		curattr |= 0x80;
	/* How to map the bright bit - we go by either */
	if ((vtink | vtpaper) & 0x10)
		curattr |= 0x40;
}
