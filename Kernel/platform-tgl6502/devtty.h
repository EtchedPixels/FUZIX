#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

extern int nc100_tty_open(uint8_t minor, uint16_t flag);
extern int nc100_tty_close(uint8_t minor);
extern void nc100_tty_init(void);
#endif
