#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

#include <../include/vt.h>
#include <../include/graphics.h>

struct pty {
	unsigned char *base;	/* base of buffer in cpu space */
	unsigned char *cpos;	/* current location of cursor */
	unsigned char csave;	/* charactor that is under the cursor */
	struct vt_switch vt;	/* the vt.o module's state */
	unsigned int scrloc;	/* location to put into gimme */
	unsigned char vmod;     /* video mode */
	unsigned char vres;     /* video register settings of this tty */   
	unsigned char width;    /* text width of screen */
	unsigned char height;   /* text height */
	unsigned char right;    /* right most coord */
	unsigned char bottom;   /* bottom most coord */
	struct display *fdisp;  /* ptr to struct for ioctl */
	uint8_t attr;           /* attribute byte to apply */
};

extern struct pty *curpty;

int my_tty_close( uint8_t minor ); /* wrapper call to close DW ports */
int gfx_ioctl(uint8_t minor, uarg_t arg, char *ptr);

#endif
