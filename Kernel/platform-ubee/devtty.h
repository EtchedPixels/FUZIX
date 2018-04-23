#ifndef _DEVTTY_H
#define _DEVTTY_H

extern void tty_interrupt(void);
extern void kbd_interrupt(void);
extern void lpen_kbd_poll(void);

extern uint8_t kbscan(void);
extern uint8_t kbtest(uint16_t code);

extern uint16_t vtattrib;
extern uint16_t vtaddr;
extern uint16_t vtbase;
extern uint16_t vtcount;
extern uint8_t vtchar;

extern void vwrite(void);
extern void do_cursor_on(void);

#endif
