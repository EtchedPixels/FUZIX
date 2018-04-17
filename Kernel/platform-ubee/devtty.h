#ifndef _DEVTTY_H
#define _DEVTTY_H

extern void tty_interrupt(void);
extern void kbd_interrupt(void);
extern void lpen_kbd_poll(void);

extern uint8_t kbscan(void);
extern uint8_t kbtest(uint16_t code);
#endif
