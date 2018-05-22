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

static const struct display trsdisplay[1] = {
  {
    /* Once we get around to it this is probably best described as
       128 x 96 sixel */
    0,
    64, 16,
    64, 16,
    255, 255,
    FMT_TEXT,
    HW_UNACCEL,
    GFX_MULTIMODE|GFX_TEXT,
    2,
    0
  }
};

/* Assumes a Tandy board */
static const struct videomap trsmap = {
  0,
  0,
  0x3C00,
  0x0400,		/* Directly mapped */
  0, 0, 	/* No segmentation */
  1,		/* Standard spacing */
  MAP_FBMEM_SIMPLE|MAP_FBMEM
};

static uint8_t vmode;

int gfx_ioctl(uint8_t minor, uarg_t arg, char *ptr)
{
  uint8_t m;
  int err;

  if (arg >> 8 != 0x03)
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
  /* Users can "map" 8) the MMIO into their process and use the
     card directly */
  case GFXIOC_MAP:
    if (vmode == 0)
      break;
    return uput(&trsmap, ptr, sizeof(trsmap));
  }
  return -1;
}
