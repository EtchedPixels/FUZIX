/*
 *	Graphics logic for the TRS80 graphics add on board
 *
 *	FIXME: GETPIXEL, direct raw I/O access, scrolling, rects and
 *	aligned blit are probably the basic set we should go for
 */

#include <kernel.h>
#include <kdata.h>
#include <vt.h>
#include <graphics.h>
#include <devgfx.h>

static const struct display trsdisplay = {
  640, 240,
  1024, 256,
  1, 1,		/* Need adding to ioctls */
  FMT_MONO_BW,
  HW_TRS80GFX,
  GFX_ENABLE|GFX_MAPPABLE|GFX_OFFSCREEN,
  32,
  GFX_SETPIXEL,
  0
};

/* Assumes a Tandy board */
static const struct videomap trsmap = {
  0,
  0x80, 	/* I/O ports.. 80-83 + 8C-8E */
  0, 0,		/* Not mapped into main memory */
  0, 0, 	/* No segmentation */
  1,		/* Standard spacing */
  MAP_PIO
};

uint16_t video_op[GFX_BUFLEN];

__sfr __at 0x83 gfx_ctrl;

struct attribute video_attr;	/* Shared with asm code */
 
int gfx_ioctl(uint8_t minor, uarg_t arg, char *ptr)
{
  if (arg >> 8 != 0x03)
    return vt_ioctl(minor, arg, ptr);
  switch(arg) {
  case GFXIOC_GETINFO:
    return uput(&trsdisplay, ptr, sizeof(trsdisplay));
  case GFXIOC_ENABLE:
    gfx_ctrl = 3;	/* we might want 1 for special cases */
    return 0;
  case GFXIOC_DISABLE:
    gfx_ctrl = 0;
  case GFXIOC_UNMAP:
    return 0;
  /* Users can "map" 8) the I/O ports into their process and use the
     card directly */
  case GFXIOC_MAP:
    return uput(&trsmap, ptr, sizeof(trsmap));
  case GFXIOC_SETATTR:
    return uget(&video_attr, ptr, sizeof(video_attr));
  case GFXIOC_SETPIXEL:
    if (uget(&video_op, ptr, sizeof(video_op)))
      return -1;
    video_setpixel();
    return 0;
  default:
    udata.u_error = EINVAL;
    return -1;
  }
}
