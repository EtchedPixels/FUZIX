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
#include "genie.h"

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
  /* TODO graphics mode */
};

/* TODO: Arbitrate graphics between tty 1 and tty 2 */
int gfx_ioctl(uint8_t minor, uarg_t arg, char *ptr)
{
  uint8_t m;

  if (minor > 2 || (arg >> 8 != 0x03))
    return vt_ioctl(minor, arg, ptr);

  switch(arg) {
  case GFXIOC_GETINFO:
    return uput(&trsdisplay[video_mode], ptr, sizeof(struct display));
  }
  return -1;
}

void gfx_init(void)
{
}

