/*
 *	Graphics logic for the NC100/200
 */

#include <kernel.h>
#include <kdata.h>
#include <vt.h>
#include <graphics.h>
#include <devgfx.h>

#ifdef CONFIG_NC100
static struct display ncdisplay = {
  480, 64,
  512, 64,
  0xFF, 0xFF,		/* For now */
  FMT_MONO_WB,
  HW_UNACCEL,
  0,
  0,
  GFX_SETPIXEL|GFX_RAW|GFX_RAWCOPY,
  0
};
#else
static struct display ncdisplay = {
  480, 128,
  512, 128,
  0xFF, 0xFF,		/* For now */
  FMT_MONO_WB,
  HW_UNACCEL,
  0,
  0,
  GFX_SETPIXEL|GFX_RAW|GFX_RAWCOPY,
  0
};
#endif

extern uint16_t video_op[GFX_BUFLEN];

extern struct attribute video_attr;	/* Shared with asm code */
 
int gfx_ioctl(uint8_t minor, uarg_t arg, char *ptr)
{
  if (arg >> 8 != 0x03)
    return vt_ioctl(minor, arg, ptr);
  if (arg == GFXIOC_GETINFO)
    return uput(&ncdisplay, ptr, sizeof(ncdisplay));
  if (arg == GFXIOC_SETATTR)
    return uget(&video_attr, ptr, sizeof(video_attr));
  if (uget(&video_op, ptr, sizeof(video_op)))
      return -1;
  switch(arg) {
  case GFXIOC_SETPIXEL:
    video_setpixel();
    return 0;
  case GFXIOC_CMD:
    video_cmd();
    return 0;  
  default:
    udata.u_error = EINVAL;
    return -1;
  }
}
