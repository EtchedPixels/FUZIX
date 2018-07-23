#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

#define KEY_ROWS 8
#define KEY_COLS 7
extern uint8_t keymap[8];
extern uint8_t keyboard[8][7];
extern uint8_t shiftkeyboard[8][7];

extern const signed char vt_tright[2];
extern const signed char vt_tbottom[2];
extern uint8_t curtty;

extern int gfx_ioctl(uint8_t minor, uarg_t arg, char *ptr);

extern void video_cmd(uint8_t *ptr);
extern void video_read(uint8_t *ptr);
extern void video_write(uint8_t *ptr);

int my_tty_close( uint8_t minor ); /* wrapper call to close DW ports */
#endif
