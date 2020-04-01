/*
 *	Graphics logic for the NC100/200
 */

#include <kernel.h>
#include <kdata.h>
#include <vt.h>
#include <graphics.h>
#include <devgfx.h>

#ifdef CONFIG_NC200
static struct display ncdisplay = {
  0,
  480, 128,
  512, 128,
  0xFF, 0xFF,		/* For now */
  FMT_MONO_WB,
  HW_UNACCEL,
  GFX_TEXT,
  0,
  GFX_DRAW,
};
#else
static struct display ncdisplay = {
  0,
  480, 64,
  512, 64,
  0xFF, 0xFF,		/* For now */
  FMT_MONO_WB,
  HW_UNACCEL,
  GFX_TEXT,
  0,
  GFX_DRAW
};
#endif

int gfx_ioctl(uint_fast8_t minor, uarg_t arg, char *ptr)
{
  uint8_t *tmp;
  uint16_t l;
  if (minor != 1 || (arg >> 8 != 0x03))
    return vt_ioctl(minor, arg, ptr);

  switch(arg) {
  case GFXIOC_GETINFO:
    return uput(&ncdisplay, ptr, sizeof(ncdisplay));
  case GFXIOC_DRAW:
    /* Note: we assume we will not map the screen over the buffers */
    tmp = (uint8_t *)tmpbuf();
    l = ugetw(ptr);
    if (l < 6 || l > 512)
      goto bad;
    if (uget(ptr + 2, tmp, l))
      goto bad2;
    /* TODO
    if (draw_validate(ptr, l, 480, 64))
      goto bad; */
    video_cmd(tmp);
    tmpfree(tmp);
    return 0;
  default:
    udata.u_error = EINVAL;
    return -1;
  }
bad:
  udata.u_error = EINVAL;
bad2:
  brelse((bufptr) tmp);
  return -1;
}
