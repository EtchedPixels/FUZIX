#ifndef _DEV_GFX_H
#define _DEV_GFX_H

#include <graphics.h>

extern int gfx_ioctl(uint8_t minor, uarg_t arg, char *ptr);
extern void video_cmd(uint8_t *ptr);
extern void video_read(uint8_t *ptr);
extern void video_write(uint8_t *ptr);
extern void video_exg(uint8_t *ptr);

#endif
