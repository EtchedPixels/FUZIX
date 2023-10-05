#ifndef _DEV_GFX_H
#define _DEV_GFX_H

#include <graphics.h>

extern int gfx_ioctl(uint8_t minor, uarg_t arg, char *ptr);
extern void gfx_init(void);

extern uint8_t video_mode;
extern uint8_t has_hrg1;
extern uint8_t has_chroma;

#endif
