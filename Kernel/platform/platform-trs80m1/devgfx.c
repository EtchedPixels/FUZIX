/*
 *	Graphics logic for the TRS80 model I, III and friends
 *
 *	- Need to figure out how to interact with console switches
 *	- Need to add LNW80 graphics modes
 */

#include <kernel.h>
#include <kdata.h>
#include <tty.h>
#include <vt.h>
#include <graphics.h>
#include <devgfx.h>
#include "trs80.h"

__sfr __at 0x00 hrg_off;
__sfr __at 0x04 hrg_data;
__sfr __at 0x79 vdps;
__sfr __at 0x7C chromajs0;
__sfr __at 0x82 gfx_data;
__sfr __at 0x83 gfx_ctrl;
__sfr __at 0xEC le18_data;
__sfr __at 0xEF le18_ctrl;
__sfr __at 0xFF ioctrl;

uint8_t trs80_udg;

uint8_t has_hrg1;
uint8_t has_chroma;	/* and thus 2 joystick ports */
uint8_t video_mode;
static uint8_t max_mode = 0;

static struct display trsdisplay[6] = {
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
    FMT_MONO_WB,
    HW_TRS80GFX,
    GFX_MULTIMODE|GFX_MAPPABLE,	/* No overlay control on Model 3 */
    32,
    0			/* For now lets just worry about map */
  },
  /* Microlabs Grafyx */
  {
    1,
    512, 192,
    512, 192,
    255, 255,
    FMT_MONO_WB,
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
    FMT_MONO_WB,
    HW_HRG1B,
    GFX_MULTIMODE|GFX_MAPPABLE|GFX_TEXT,
    9,
    0
  },
  /* CHROMAtrs */
  {
    1,
    256, 192,
    256, 192,
    255, 255,
    FMT_VDP,
    HW_VDP_9918A,
    GFX_MULTIMODE|GFX_MAPPABLE,	/* We don't support it as a console yet */
    16,
    0
  },
  /* Lowe Electronics LE-18 */
  {
    1,
    384, 192,
    384, 192,
    255, 255,
    FMT_MONO_WB,
    HW_LOWE_LE18,
    GFX_MULTIMODE|GFX_MAPPABLE|GFX_TEXT,
    16,
    0
  }
};

static struct videomap trsmap[6] = {
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
  /* CHROMAtrs */
  {
    0,
    0x78,	/* I/O ports 0x78/79 */
    0, 0,
    0, 0,
    1,
    MAP_PIO	/* VDP is I/O mapped */
  },
  /* Lowe Electronics LE-18 */
  {
    0,
    0xEC,	/* I/O ports 0xEC-0xEF */
    0, 0,
    0, 0,
    1,
    MAP_PIO	/* VDP is I/O mapped */
  }
};

static uint8_t displaymap[4] = {0, 0, 0, 0};

static int8_t udg_ioctl(uarg_t arg, char *ptr);

/* TODO: Arbitrate graphics between tty 1 and tty 2 */
int gfx_ioctl(uint8_t minor, uarg_t arg, char *ptr)
{
  uint8_t m;

  if (minor <= 2 && (arg == VTFONTINFO || arg == VTSETFONT || arg == VTSETUDG))
    return udg_ioctl(arg, ptr);

  if (minor > 2 || (arg >> 8 != 0x03))
    return vt_ioctl(minor, arg, ptr);

  switch(arg) {
  case GFXIOC_GETINFO:
    return uput(&trsdisplay[video_mode], ptr, sizeof(struct display));
  case GFXIOC_GETMODE:
  case GFXIOC_SETMODE:
    m = ugetc(ptr);
    if (m > max_mode)
      break;
    m = displaymap[m];
    if (arg == GFXIOC_GETMODE)
      return uput(&trsdisplay[m], ptr, sizeof(struct display));
    video_mode = m;
    /* Going back to text mode we need to turn off graphics cards. Going the
       other way we let the applications handle it as they may want to do
       memory wipes and the like first */
    if (video_mode == 0) {
      if (displaymap[1] == 1)
        gfx_ctrl = 0;
      else if (displaymap[1] == 2)
        ioctrl = 0x20;
      else if (displaymap[1] == 3)
        hrg_off = 1;
      else if (displaymap[1] == 5)
        le18_ctrl = 0;
    }
    return 0;
  case GFXIOC_UNMAP:
    return 0;
  /* Users can "map" 8) the framebuffer into their process and use the
     card directly */
  case GFXIOC_MAP:
    return uput(&trsmap[video_mode], ptr, sizeof(struct videomap));
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
      has_hrg1 = 1;
    }
    /* LE-18 */
    if(trs80_model == VIDEOGENIE && le18_data != 0xFF) {
      displaymap[++max_mode] = 5;
      trsdisplay[5].mode = max_mode;
    }
  } else if (trs80_model == TRS80_MODEL3) {
    /* The model 3 might have an 80-Grafix UDG card, or a Graphyx
       or a Tandy card */
    uint8_t *fb = (uint8_t *)0x3C00;
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
    ioctrl = 0x20;
    *fb = c;
    if (max_mode == 0 && gfx_data != 0xFF) {
      max_mode = 1;
      displaymap[1] = 1;
    }
  }
  /* It's possible to have a CHROMAtrs and other adapters as it fits
     on the external bus. 70-7C is also a common location for RTC clocks
     which makes detection trickier. The clock will show 0 in the upper
     bits, the joystick port will not */
  if (vdps != 0xFF && (chromajs0 & 0xC0) == 0xC0) {
    displaymap[++max_mode] = 4;
    trsdisplay[4].mode = max_mode;
    has_chroma = 1;
  }
}

/*
 *	UDG drivers
 *
 *	We don't currently address UDG fitted without lower case. That gets
 *	deeply weird because you get disjoint ranges with one bit magically
 *	invented by logic. For the UDG only ones we just halve the range, but
 *	for the PCG80 what about full font ?
 *
 *	(The Tandy (Microfirma) one requires lowercase is fitted according
 *	to the manual)
 *
 *	One other corner case to consider is that if you have no lower case
 *	we ought to requisition the UDG and load a font into it then use
 *	128-159 for lower case.
 */

static struct fontinfo fonti[4] = {
  { 0, },
  { 0, 255, 0, 255, FONT_INFO_6X12P16 },
  { 128, 191, 128, 191, FONT_INFO_6X12P16 },
  { 128, 255, 128, 255, FONT_INFO_6X12P16 }
};

__sfr __at 130 trshg_nowrite;
__sfr __at 140 trshg_write;
__sfr __at 150 trshg_gfxoff;
__sfr __at 155 trshg_gfxon;
__sfr __at 0xFE pcg80;
__sfr __at 0xFF p80gfx;

static uint8_t old_pcg80;
static uint8_t old_p80gfx;
static uint8_t udgflag;		/* So we know what fonts are loaded */
#define SOFTFONT_UDG	1
#define SOFTFONT_ALL	2

static void load_char_pcg80(uint8_t ch, uint8_t *cdata)
{
  uint8_t bank = ch >> 6;
  uint8_t *addr = (uint8_t *)0x3C00 + ((ch & 0x3F) << 4);
  uint8_t i;

  pcg80 = old_pcg80 | 0x60 | bank;	/* Programming mode on */

  for (i = 0; i < 16; i++)
    *addr++ = *cdata++ << 1 | 0x80;
  pcg80 = old_pcg80;
}

static void load_char_80gfx(uint8_t ch, uint8_t *cdata)
{
  uint8_t *addr = (uint8_t *)0x3C00 + ((ch & 0x3F) << 4);
  uint8_t i;

  p80gfx = old_p80gfx | 0x60;
  for (i = 0; i < 16; i++)
    *addr++ = *cdata++ << 1 | 0x80;
  p80gfx = old_p80gfx;
}

static void load_char_trs(uint8_t ch, uint8_t *cdata)
{
  uint8_t *addr = (uint8_t *)0x3C00 + ((ch & 0x3F) << 4);
  uint8_t i;

  trshg_write = 1;
  for (i = 0; i < 16; i++)
    *addr++ = *cdata++ << 1 | 0x80;
  trshg_nowrite = 1;
}

void (*load_char[4])(uint8_t, uint8_t *) = {
  NULL,
  load_char_pcg80,
  load_char_80gfx,
  load_char_trs,
};

static void udg_config(void)
{
  switch(trs80_udg) {
  case UDG_PCG80:
    if (udgflag & SOFTFONT_UDG)
      old_pcg80 |= 0x80;	/* 128-255 soft font */
    if (udgflag & SOFTFONT_ALL)
      old_pcg80 |= 0x88;
    pcg80 = old_pcg80 | 0x20;
    break;
  case UDG_80GFX:
    if (udgflag & SOFTFONT_UDG)
      old_p80gfx |= 0x80;
    p80gfx = old_p80gfx | 0x20;
    break;
  case UDG_MICROFIRMA:
    if (udgflag & SOFTFONT_UDG)
      trshg_gfxon = 1;
    break;
  }
}

static int8_t udg_ioctl(uarg_t arg, char *ptr)
{
  uint8_t base = fonti[trs80_udg].font_low;
  uint8_t limit = fonti[trs80_udg].font_high;
  int i;
  uint8_t c;

  /* Not supported */
  if (trs80_udg == UDG_NONE) {
    udata.u_error = EOPNOTSUPP;
    return -1;
  }
  /* No lower case available */
  if (!video_lower)
    limit = 191;

  switch(arg) {
  case VTFONTINFO:
    return uput(fonti + trs80_udg, ptr, sizeof(struct fontinfo));
  case VTSETUDG:
    base = fonti[trs80_udg].udg_low;
    limit = fonti[trs80_udg].udg_high;
    c = ugetc(ptr);
    ptr++;
    if (c < base || c > limit) {
      udata.u_error = EINVAL;
      return -1;
    }
    base = c;
    limit = c + 1;
    /* Fall through */
  case VTSETFONT:
    for (i = base; i <= limit; i++) {
      uint8_t c[16];
      if (uget(c, ptr, 16) == -1)
        return -1;
      ptr += 16;
      load_char[trs80_udg](i, c);
    }
    if (arg == VTSETUDG)
      udgflag |= SOFTFONT_UDG;
    else
      udgflag |= SOFTFONT_ALL;
    udg_config();
    return 0;
  }
  return -1;
}
