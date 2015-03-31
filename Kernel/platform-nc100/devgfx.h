#ifndef _DEV_GFX_H
#define _DEV_GFX_H

#include <graphics.h>

extern int gfx_ioctl(uint8_t minor, uarg_t arg, char *ptr);
extern struct attribute video_attr;
extern uint16_t video_op[GFX_BUFLEN];

extern void video_setpixel(void);
extern void video_cmd(void);

#endif
