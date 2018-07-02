/*
 *	Graphics logic for the TRS80 graphics add on board
 *
 *	FIXME: - turn the gfx on/off when we switch tty
 *	       - tie gfx to one tty and report EBUSY for the other on enable
 */

#include <kernel.h>
#include <kdata.h>
#include <vt.h>
#include <graphics.h>
#include <devgfx.h>

static const struct display trsdisplay[2] = {
  {
    /* Once we get around to it this is probably best described as
       160 x 72 sixel */
    0,
    80, 24,
    80, 24,
    255, 255,
    FMT_TEXT,
    HW_UNACCEL,
    GFX_MULTIMODE|GFX_TEXT,
    2,
    0
  },
  {
    1,
    640, 240,
    1024, 256,
    1, 1,		/* Need adding to ioctls */
    FMT_MONO_BW,
    HW_TRS80GFX,
    GFX_MULTIMODE|GFX_MAPPABLE|GFX_OFFSCREEN,	/* Can in theory do pans */
    32,
    GFX_DRAW|GFX_READ|GFX_WRITE
  }
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

__sfr __at 0x83 gfx_ctrl;

static uint8_t vmode;

static int gfx_draw_op(uarg_t arg, char *ptr, uint8_t *buf)
{
  int l;
  int c = 8;	/* 4 x uint16_t */
  uint16_t *p = (uint16_t *)buf;
  l = ugetw(ptr);
  if (l < 6 || l > 512)
    return EINVAL;
  if (arg != GFXIOC_READ)
    c = l;
  if (uget(buf, ptr + 2, c))
    return EFAULT;
  switch(arg) {
  case GFXIOC_DRAW:
    /* TODO
    if (draw_validate(ptr, l, 1024, 256))
      return EINVAL */
    video_cmd(buf);
    break;
  case GFXIOC_WRITE:
  case GFXIOC_READ:
  case GFXIOC_EXG:
    if (l < 8)
      return EINVAL;
    l -= 8;
    if (p[0] > 128 || p[1] > 255 || p[2] > 128 || p[3] > 255 ||
      p[0] + p[2] > 128 || p[1] + p[3] > 255 ||
      (p[2] * p[3]) > l)
      return -EFAULT;
    if (arg == GFXIOC_WRITE)
      video_write(buf);
    else {
      if (arg == GFXIOC_READ)
        video_read(buf);
      else
        video_exg(buf);
      if (uput(buf + 10, ptr, l - 2))
        return EFAULT;
      return 0;
    }
  }
  return 0;
}

int gfx_ioctl(uint8_t minor, uarg_t arg, char *ptr)
{
  uint8_t m;
  int err;

  if (minor > 2 || (arg >> 8 != 0x03))
    return vt_ioctl(minor, arg, ptr);

  switch(arg) {
  case GFXIOC_GETINFO:
    return uput(&trsdisplay[vmode], ptr, sizeof(struct display));
  case GFXIOC_GETMODE:
  case GFXIOC_SETMODE:
    m = ugetc(ptr);
    if (m > 1)
      break;
    if (arg == GFXIOC_GETMODE)
      return uput(&trsdisplay[m], ptr, sizeof(struct display));
    vmode = m;
    gfx_ctrl = m ? 3 : 0;	/* we might want 1 for special cases */
    return 0;
  case GFXIOC_UNMAP:
    return 0;
  /* Users can "map" 8) the I/O ports into their process and use the
     card directly */
  case GFXIOC_MAP:
    if (vmode == 0)
      break;
    return uput(&trsmap, ptr, sizeof(trsmap));
  case GFXIOC_DRAW:
  case GFXIOC_READ:
  case GFXIOC_WRITE:
  case GFXIOC_EXG:
    if (vmode == 1) {
      uint8_t *tmp = (uint8_t *)tmpbuf();
      err = gfx_draw_op(arg, ptr, tmp);
      tmpfree(tmp);
      if (err) {
        udata.u_error = err;
        err = -1;
       }
       return err;
    }
    /* Fall through */
  }
  return -1;
}
