#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <device.h>
#include <vt.h>
#include <tty.h>
#include <graphics.h>
#include <thomtty.h>


#undef  DEBUG			/* UNdefine to delete debug code sequences */

static unsigned char tbuf1[TTYSIZ];
static uint8_t vmode;

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	_CSYS,
};

/* tty1 is the screen. Serial can come later */

/* Output for the system console (kprintf etc) */
void kputchar(uint8_t c)
{
	if (c == '\n')
		tty_putc(1, '\r');
	tty_putc(1, c);
}

ttyready_t tty_writeready(uint8_t minor)
{
	return TTY_READY_NOW;
}

void tty_putc(uint8_t minor, unsigned char c)
{
	/* For now we only support video out in the base mode */
	if (vmode == 0)
		vtoutput(&c, 1);
}

void tty_sleeping(uint8_t minor)
{
    used(minor);
}

void tty_setup(uint8_t minor, uint8_t flags)
{
}

int tty_carrier(uint8_t minor)
{
	return 1;
}

void tty_data_consumed(uint8_t minor)
{
}

void poll_keyboard(void)
{
	uint8_t c;
	/* Monitor call wrapped. Returns value from B in monitor */
	/* Needs a bit of work for non trivial cases */
	if ((c =  mon_keyboard()) != 0)
		tty_inproc(1, c);
}

/* Not used for yet */
void plt_reinterrupt(void)
{
	panic("reint");
}

/* TO9 only gets 8K mode */

#ifdef CONFIG_TO9
#define NUM_MODES	1
static const struct display display[] = {
{
		/* 320x200 mono 8MHz */
		0,
		320, 200,
		320, 200,
		FMT_MONO_WB,	/* mono packed pixel */
		HW_UNACCEL,
		GFX_TEXT|GFX_MAPPABLE|GFX_VBLANK|GFX_MULTIMODE,
		0,
		0,	/* TODO: draw/write/etc */
		40, 25
	}
};

#else
#define NUM_MODES	5
static const struct display display[] = {
	{
		/* 80 column, 640x200 mono bitmap ~16MHz */
		0,
		640, 200,
		640, 200,
		0xFF, 0xFF,
		FMT_MONO_WB,
		HW_UNACCEL,
		GFX_TEXT|GFX_MAPPABLE|GFX_VBLANK|GFX_MULTIMODE,
		0,
		0,	/* TODO: draw/write/etc */
		80, 25
	},
	{
		/* 320x200 four colour 8MHz */
		/* Should also be easy for text - write to planes by ink */
		1,
		320, 200,
		320, 200,
		0xFF, 0xFF,
		FMT_PLANAR2,	/* Two plane */
		HW_UNACCEL,
		GFX_MAPPABLE|GFX_VBLANK|GFX_MULTIMODE,
		0,
		0,	/* TODO: draw/write/etc */
		40, 25
	},
	{
		/* 320x200 16 colour 16MHz */
		2,
		320, 200,
		320, 200,
		0xFF, 0xFF,
		FMT_THOMSON_C16,	/* 4bit packed pixel interleaved */
		HW_UNACCEL,
		GFX_MAPPABLE|GFX_VBLANK|GFX_MULTIMODE,
		0,
		0,	/* TODO: draw/write/etc */
		40, 25
	},
	{
		/* 320x200 16 colour 8MHz, 2 colours per byte */
		/* FIXME: should be easy for text mode */
		3,
		320, 200,
		320, 200,
		0xFF, 0xFF,
		FMT_THOMSON_TO7,
		HW_UNACCEL,
		GFX_MAPPABLE|GFX_VBLANK|GFX_MULTIMODE,
		0,
		0,	/* TODO: draw/write/etc */
	},
	{
		/* 320x200 mono 8MHz */
		/* FIXME: at least text for this mode */
		4,
		320, 200,
		320, 200,
		0xFF, 0xFF,
		FMT_MONO_WB,	/* mono packed pixel */
		HW_UNACCEL,
		GFX_MAPPABLE|GFX_VBLANK|GFX_MULTIMODE,
		0,
		0,	/* TODO: draw/write/etc */
		40, 25
	}
	/* There are then some weird overlay mdoes we ignore */
};

static uint8_t videobyte[5] = {
	0x2A,	/* 80 column, 16MHz */
	0x11,	/* 40 column, bitmap 4, 8MHz */
	0x7B,	/* 40 column, bitmap 16, 4MHz */
	0x00,	/* 40 column, to7, 8MHz */
	0x24	/* 40 column 320x200 mono (8K) */
};

static volatile uint8_t *video = (uint8_t *)0xE7DC;

#endif

int gfx_ioctl(uint_fast8_t minor, uarg_t request, char *ptr)
{
	if (minor != 1)
		return tty_ioctl(minor, request, ptr);
	switch(request) {
	case GFXIOC_GETINFO:
		return uput(&display[vmode], ptr, sizeof(struct display));
#ifdef VIDMAP_BASE
	case GFXIOC_MAP: {
		static const struct videomap displaymap = {
			VIDMAP_BASE,
			0,
			16384,
			16384,
			0,
			0,
			0,
			MAP_MMIO|MAP_FBMEM|MAP_FBMEM_SIMPLE
		};

		return uput(&displaymap, ptr, sizeof(displaymap));
	}
	case GFXIOC_UNMAP:
		return 0;
#endif
#if NUM_MODES > 1
	case GFXIOC_GETMODE:
	case GFXIOC_SETMODE:
	{
		uint8_t m = ugetc(ptr);
		if (m > 4) {
			udata.u_error = EINVAL;
			return -1;
		}
		if (request == GFXIOC_GETMODE)
			return uput(&display[m], ptr, sizeof(struct display));
		vmode = m;
		*video = videobyte[m];
		return 0;
	}
#endif
	case GFXIOC_WAITVB:
		vblank_wait++;
		psleep(&vblank_wait);
		vblank_wait--;
		chksigs();
		if (udata.u_cursig) {
			udata.u_error = EINTR;
				return -1;
		}
		return 0;
	}
	/* TODO: palette, ink, paper, border etc */
	return vt_ioctl(minor, request, ptr);
}
