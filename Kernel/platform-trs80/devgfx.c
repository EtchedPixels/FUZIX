/*
 *	Graphics logic for the TRS80 graphics add on board
 *
 *	FIXME: - turn the gfx on/off when we switch tty
 *	       - tie gfx to one tty and report EBUSY for the other on enable
 *
 *	TODO: 64 column
 */

#include <kernel.h>
#include <kdata.h>
#include <vt.h>
#include <graphics.h>
#include <devtty.h>
#include <devgfx.h>

static const struct display trsdisplay[3] = {
	{
	 /* Once we get around to it this is probably best described as
	    160 x 72 sixel */
	 0,
	 160, 72,
	 80, 24,
	 255, 255,
	 FMT_6PIXEL_128,
	 HW_UNACCEL,
	 GFX_MULTIMODE | GFX_TEXT,
	 2,
	 0 },
	{
	 1,
	 640, 240,
	 1024, 256,
	 8, 1,			/* Need adding to ioctls */
	 FMT_MONO_BW,
	 HW_TRS80GFX,
	 GFX_MULTIMODE | GFX_MAPPABLE | GFX_OFFSCREEN,	/* Can do pans */
	 32,
	 GFX_SCROLL,
	 80, 24,
	  },
	{
	 1,
	 640, 240,
	 1024, 256,
	 0xFF, 0xFF,		/* Need adding to ioctls */
	 FMT_MONO_BW,
	 HW_TRS80GFX,
	 GFX_MULTIMODE | GFX_MAPPABLE,
	 32,
	 0,
	 80, 24 }
	/* FIXME: Need to add Micrographyx at some point (needs a different id to
	   the TRS80 model III one */
};

/* Assumes a Tandy board */
static const struct videomap trsmap = {
	0,
	0x80,			/* I/O ports.. 80-83 + 8C-8E */
	0, 0,			/* Not mapped into main memory */
	0, 0,			/* No segmentation */
	1,			/* Standard spacing */
	MAP_PIO
};


__sfr __at 0x83 gfx_ctrl;
__sfr __at 0x8C gfx_xpan;
__sfr __at 0x8D gfx_ypan;
__sfr __at 0x8E gfx_xor;

static uint8_t vmode;

int gfx_ioctl(uint8_t minor, uarg_t arg, char *ptr)
{
	uint8_t m;
	int err;

	if (minor > 2 || (arg >> 8 != 0x03))
		return vt_ioctl(minor, arg, ptr);

	switch (arg) {
	case GFXIOC_GETINFO:
		return uput(&trsdisplay[vmode], ptr,
			    sizeof(struct display));
	case GFXIOC_GETMODE:
	case GFXIOC_SETMODE:
		m = ugetc(ptr);
		if (m > 1 || gfxtype == 0)
			break;
		if (m)
			m = gfxtype;
		if (arg == GFXIOC_GETMODE)
			return uput(&trsdisplay[m], ptr,
				    sizeof(struct display));
		vmode = m;
		if (gfxtype == 1) {
			gfx_xpan = 0;
			gfx_ypan = 0;
			gfx_xor = 1;
			gfx_ctrl = m ? 3 : 0;	/* we might want 1 for special cases */
		} else {
			gfx_ctrl = m ? 1 : 0;
		}
		return 0;
	case GFXIOC_UNMAP:
		return 0;
		/* Users can "map" 8) the I/O ports into their process and use the
		   card directly */
	case GFXIOC_MAP:
		if (vmode == 0)
			break;
		return uput(&trsmap, ptr, sizeof(trsmap));
	case GFX_SCROLL:
		if (vmode == 1) {
			gfx_xpan = ugetw(ptr) & 127;
			gfx_ypan = ugetw(ptr + 2) & 255;
			return 0;
		}
		break;
	}
	return -1;
}
