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

extern int gfx_ioctl(uint8_t minor, uarg_t arg, char *ptr);

extern void map_video_font(void);
extern void unmap_video_font(void);

extern void ctc_load(uint8_t *map);
extern uint8_t ctc6545[48];
extern void video_40(void);
extern void video_80(void);

#endif
