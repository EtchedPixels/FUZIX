#ifndef _DEV_GFX_H
#define _DEV_GFX_H

#include <graphics.h>

extern int gfx_ioctl(uint8_t minor, uarg_t arg, char *ptr);
extern void video_cmd(uint8_t *ptr);

#endif
