#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

void tty_putc(uint_fast8_t minor, uint_fast8_t c);
void tty_pollirq(void);
int rctty_open(uint_fast8_t minor, uint16_t flag);
int rctty_ioctl(uint8_t minor, uarg_t arg, char *ptr);

extern uint8_t shadowcon;
extern uint8_t nuart;


#endif
