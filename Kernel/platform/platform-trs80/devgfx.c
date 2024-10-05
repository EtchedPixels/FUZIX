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
	 0,
	 80,24
	 }, {
	 1,
	 640, 240,
	 1024, 256,
	 8, 1,			/* Need adding to ioctls */
	 FMT_MONO_BW,
	 HW_TRS80GFX,
	 /* The tandy one has offscreen space and can pan */
	 GFX_MULTIMODE | GFX_MAPPABLE | GFX_OFFSCREEN | GFX_READ | GFX_WRITE | GFX_DRAW | GFX_EXG | GFX_BLIT,
	 32,
	 GFX_SCROLL,
	 80, 24,
	}, {
	 1,
	 640, 240,
	 1024, 256,
	 0xFF, 0xFF,		/* Need adding to ioctls */
	 FMT_MONO_BW,
	 HW_TRS80GFX,
	 GFX_MULTIMODE | GFX_MAPPABLE | GFX_READ | GFX_WRITE | GFX_DRAW | GFX_EXG | GFX_BLIT,
	 32,
	 0,
	 80, 24
	}
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
uint8_t ctrl_cache;

extern unsigned gfx_blit(struct blit *blit) __z88dk_fastcall;
extern unsigned gfx_draw(uint8_t *ptr) __z88dk_fastcall;
extern unsigned gfx_exg(uint8_t *ptr) __z88dk_fastcall;
extern unsigned gfx_read(uint8_t *ptr) __z88dk_fastcall;
extern unsigned gfx_write(uint8_t *ptr) __z88dk_fastcall;

static uint16_t gfx_valid(uint8_t *ptr)
{
	unsigned x = ugetw(ptr);
	unsigned y = ugetw(ptr + 2);
	unsigned w = ugetw(ptr + 4);
	unsigned h = ugetw(ptr + 6);
	if (gfxtype == 0)
		return -1;
	if (x > 79 || y > 239 || h > 240 || w > 80 ||
		x + w > 80 || y + h > 240) {
		udata.u_error = EINVAL;
		return 0;
	}
	return w * h + 8;
}

int gfx_ioctl(uint8_t minor, uarg_t arg, char *ptr)
{
	uint8_t m;
	uint16_t len;
	struct blit blit;

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
			ctrl_cache = m ? 3 : 0;	/* we might want 1 for special cases */
		} else {
			ctrl_cache = m ? 1 : 0;
		}
		gfx_ctrl = ctrl_cache;
		return 0;
	case GFX_WRITE:
		len = gfx_valid(ptr);
		if (len == 0 || valaddr(ptr, len, 0))
			return -1;
		return gfx_write(ptr);
	case GFX_READ:
		len = gfx_valid(ptr);
		if (len == 0 || valaddr(ptr, len, 1))
			return -1;
		return gfx_read(ptr);
	case GFX_EXG:
		len = gfx_valid(ptr);
		if (len == 0 || valaddr(ptr, len, 1))
			return -1;
		return gfx_exg(ptr);
	case GFX_DRAW:
		if (gfxtype == 0)
			return -1;
		len = ugetw(ptr);
		if (valaddr(ptr, len, 0))
			return -1;
		if (ugetw(ptr + 2) > 239 || ugetw(ptr + 4) > 79) {
			udata.u_error = ERANGE;
			return -1;
		}
		return gfx_draw(ptr);
	case GFX_BLIT:
		if (uget(ptr, &blit, sizeof(struct blit)))
			return -1;
		if (blit.xs > 127 || blit.xd > 127 || blit.ys > 256 || blit.yd > 256 || 
			blit.height > 255 || blit.width > 128 || blit.height == 0 || blit.width == 0) {
			udata.u_error = ERANGE;
			return -1;
		}
		return gfx_blit(&blit);
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
