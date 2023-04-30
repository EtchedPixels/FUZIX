#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

#define KEY_ROWS 8
#define KEY_COLS 7
extern uint8_t keymap[8];
extern uint8_t keyboard[8][7];
extern uint8_t shiftkeyboard[8][7];

#include <../include/vt.h>
#include <../include/graphics.h>

struct tty_coco3 {
	unsigned char *base;	/* base of buffer in cpu space */
	unsigned char *cpos;	/* current location of cursor */
	unsigned char csave;	/* charactor that is under the cursor */
	struct vt_switch vt;	/* the vt.o module's state */
	unsigned int scrloc;	/* location to put into gimme */
	unsigned char vmod;	/* video mode */
	unsigned char vres;	/* video register settings of this tty */
	unsigned char width;	/* text width of screen */
	unsigned char height;	/* text height */
	unsigned char right;	/* right most coord */
	unsigned char bottom;	/* bottom most coord */
	struct display *fdisp;	/* ptr to struct for ioctl */
};

extern struct tty_coco3 *curtty;
extern uint8_t curattr;

int my_tty_close(uint8_t minor);	/* wrapper call to close DW ports */
int gfx_ioctl(uint8_t minor, uarg_t arg, char *ptr);
void set_defmode(char *s);

#endif
