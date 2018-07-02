/*
 *	Graphics logic for the TRS80 2x3 block graphics
 *
 *	We don't yet address the high res board for the Model III (and I ?)
 */

#include <kernel.h>
#include <kdata.h>
#include <vt.h>
#include <graphics.h>
#include <devgfx.h>

static const struct display trsdisplay[1] = {
  {
    0,
    128, 48,
    64, 16,
    255, 255,
    FMT_6PIXEL_128,
    HW_UNACCEL,
    GFX_MAPPABLE|GFX_MULTIMODE|GFX_TEXT,
    1,
    0
  }
};

/* Assumes a Tandy board */
static const struct videomap trsmap = {
  0,
  0,
  0x3C00,
  0x0400,	/* Directly mapped */
  0, 0, 	/* No segmentation */
  1,		/* Standard spacing */
  MAP_FBMEM_SIMPLE|MAP_FBMEM
};

static uint8_t vmode;

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
    if (m != 0)
      break;
    if (arg == GFXIOC_GETMODE)
      return uput(&trsdisplay[m], ptr, sizeof(struct display));
    vmode = m;
    return 0;
  case GFXIOC_UNMAP:
    return 0;
  /* Users can "map" 8) the framebuffer into their process and use the
     card directly */
  case GFXIOC_MAP:
    if (vmode == 0)
      break;
    return uput(&trsmap, ptr, sizeof(trsmap));
  }
  return -1;
}
