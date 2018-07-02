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
#include "trs80.h"

__sfr __at 0x00 hrg_off;
__sfr __at 0x04 hrg_data;
__sfr __at 0x82 gfx_data;
__sfr __at 0x83 gfx_ctrl;
__sfr __at 0xFF ioctrl;

static uint8_t max_mode = 0;

static struct display trsdisplay[4] = {
  /* Text mode */
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
  },
  /* Tandy Graphics */
  {
    1,
    640, 240,
    640, 240,
    255, 255,
    FMT_MONO_BW,
    HW_TRS80GFX,
    GFX_MULTIMODE|GFX_MAPPABLE,
    32,
    0			/* For now lets just worry about map */
  },
  /* Microlabs Graphyx */
  {
    1,
    512, 192,
    512, 192,
    255, 255,
    FMT_MONO_BW,
    HW_MICROLABS,
    GFX_MULTIMODE|GFX_MAPPABLE,
    12,
    0
  },
  /* HRG1B */
  {
    1,
    384, 192,
    384, 192,
    255, 255,
    FMT_MONO_BW,
    HW_HRG1B,
    GFX_MULTIMODE|GFX_MAPPABLE,
    9,
    0
  }
};

static struct videomap trsmap[4] = {
  {
    0,
    0,
    0x3C00,
    0x0400,	/* Directly mapped */
    0, 0, 	/* No segmentation */
    1,		/* Standard spacing */
    MAP_FBMEM_SIMPLE|MAP_FBMEM
  },
  /* Tandy board */
  {
    0,
    0x80, 	/* I/O ports.. 80-83 + 8C-8E */
    0, 0,		/* Not mapped into main memory */
    0, 0, 	/* No segmentation */
    1,		/* Standard spacing */
    MAP_PIO
  },
  /* Microlabs */
  {
    0,
    0xFF,
    0x3C00, 0x0400,
    0, 0,
    1,
    MAP_FBMEM|MAP_PIO
  },
  /* HRG1B */
  {
    0,
    0x00, 	/* I/O ports.. 0-5 */
    0, 0,	/* Not mapped into main memory */
    0, 0, 	/* No segmentation */
    1,		/* Standard spacing */
    MAP_PIO
  },
};

static uint8_t vmode;
static uint8_t displaymap[2] = {0, 0};

/* TODO: Arbitrate graphics between tty 1 and tty 2 */
int gfx_ioctl(uint8_t minor, uarg_t arg, char *ptr)
{
  uint8_t m;

  if (minor > 2 || (arg >> 8 != 0x03))
    return vt_ioctl(minor, arg, ptr);

  switch(arg) {
  case GFXIOC_GETINFO:
    return uput(&trsdisplay[vmode], ptr, sizeof(struct display));
  case GFXIOC_GETMODE:
  case GFXIOC_SETMODE:
    m = ugetc(ptr);
    if (m > max_mode)
      break;
    m = displaymap[m];
    if (arg == GFXIOC_GETMODE)
      return uput(&trsdisplay[m], ptr, sizeof(struct display));
    vmode = m;
    /* Going back to text mode we need to turn off graphics cards. Going the
       other way we let the applications handle it as they may want to do
       memory wipes and the like first */
    if (vmode == 0) {
      if (displaymap[1] == 1)
        gfx_ctrl = 0;
      else if (displaymap[1] == 2)
        ioctrl = 0x20;
      else if (displaymap[1] == 3)
        hrg_off = 1;
    }
    return 0;
  case GFXIOC_UNMAP:
    return 0;
  /* Users can "map" 8) the framebuffer into their process and use the
     card directly */
  case GFXIOC_MAP:
    return uput(&trsmap[vmode], ptr, sizeof(struct videomap));
  }
  return -1;
}

void gfx_init(void)
{
  if (trs80_model == TRS80_MODEL1 || trs80_model == VIDEOGENIE) {
    /* HRG1B support. Might be good to also support 80-Grafix as a UDG
       module */
    if (hrg_data != 0xFF) {	/* We ought to test more carefully */
      max_mode = 1;
      displaymap[1] = 3;
    }
  } else if (trs80_model == TRS80_MODEL3) {
    /* The model 3 might have an 80-Grafix UDG card, or a Graphyx
       or a Tandy card */
    if (gfx_data != 0xFF) {
      max_mode = 1;
      displaymap[1] = 1;
    } else {
      uint8_t *fb = 0x3C00;
      uint8_t c = *fb;
      *fb = 128;
      ioctrl = 0xB2;
      if (*fb != 128) {
        /* Hopeful */
        *fb = 128;
        if (*fb == 128) { /* 80 Grafix only has 6bit wide RAM but wants
                             the top bit set on writes.. */
          displaymap[1] = 2;
          max_mode = 1;
        } /* else add UDG support FIXME */
      }
      *fb = c;
    }
  }
}
