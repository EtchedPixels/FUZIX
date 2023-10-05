#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

extern void tty_poll(void);
extern int gfx_ioctl(uint8_t minor, uarg_t arg, char *ptr);

#endif
