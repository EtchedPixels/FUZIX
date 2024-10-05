#ifndef _DEVTTY_H
#define _DEVTTY_H

extern void tty_poll(void);
extern int vga_ioctl(uint_fast8_t minor, uarg_t arg, char *ptr);


#endif
