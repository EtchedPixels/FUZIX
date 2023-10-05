#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <device.h>
#include <vt.h>
#include <tty.h>
#include <graphics.h>

extern void charprint(uint8_t ch);
extern void do_clear_bytes(void);
extern void do_cursor_on(void);

static volatile uint8_t *uart = (volatile uint8_t *)0xFE20;
static volatile uint8_t *timer = (volatile uint8_t *)0xFE10;

static char tbuf1[TTYSIZ];
static char tbuf2[TTYSIZ];
PTY_BUFFERS;

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
	PTY_QUEUES
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
	_CSYS
};

/* VT support logic */

uint16_t fb_off;
uint16_t fb_count;

uint8_t vtattr_cap;

void vtattr_notify(void)
{
}

void clear_across(int8_t y, int8_t x, int16_t ct)
{
	int i;
	fb_off = y * 640 + x;
	fb_count = ct;
	for (i = 0; i < 8; i++) {
		do_clear_bytes();
		fb_off += 80;
	}
}

void clear_lines(int8_t y, int8_t ct)
{
	fb_off = y * 640;
	fb_count = ct * 640;	/* 8 lines of pixels per line */
	do_clear_bytes();
}

void plot_char(int8_t y, int8_t x, uint16_t c)
{
	fb_off = y * 640 + x;
	charprint(c);
}

void cursor_on(int8_t y, int8_t x)
{
	fb_off = y * 640 + x;
	do_cursor_on();
}

/* Output for the system console (kprintf etc) */
void kputchar(uint8_t c)
{
	if (c == '\n')
		tty_putc(1, '\r');
	tty_putc(1, c);
}

ttyready_t tty_writeready(uint8_t minor)
{
        return TTY_READY_NOW;
}

void tty_putc(uint8_t minor, unsigned char c)
{
	if (minor == 2)
		uart[0] = c;
	else
		vtoutput(&c, 1);
}

void tty_setup(uint8_t minor, uint8_t flags)
{
}

void tty_sleeping(uint8_t minor)
{
}

/* For the moment */
int tty_carrier(uint8_t minor)
{
	return 1;
}

void tty_data_consumed(uint8_t minor)
{
}

void tty_poll(void)
{
        uint8_t x;
        
        x = uart[3] & 1;
        if (x) {
	        x = uart[2];
		tty_inproc(1, x);
	}
        x = uart[1] & 1;
        if (x) {
        	x = uart[0];
		tty_inproc(/*2*/1, x);
	}
}
                
void plt_interrupt(void)
{
	uint8_t t = *timer;
	tty_poll();
	while(t--) {
		timer_interrupt();
	}
}

/* Declare a single 640x200 mappable display that doesn't bother supporting
   kernel mode graphics helpers (because it's mappable */
static struct display display[1] = {
	{
		0,
		640, 200,
		640, 200,
		FMT_MONO_BW,
		HW_UNACCEL,
		GFX_TEXT|GFX_MAPPABLE,
		0,
		0,
	}
};

static struct videomap displaymap = {
	0,
	0,
	0,		/* Base is 0 in bank */
	16384,
	0,
	0xFE,		/* Bank $FE is framebuffer */
	0,
	MAP_FBMEM|MAP_FBMEM_SEG
};

int gfx_ioctl(uint8_t minor, uarg_t arg, char *ptr)
{
	if (minor > 1 || arg >> 8 != 0x03)
		return vt_ioctl(minor, arg, ptr);
	switch(arg) {
	case GFXIOC_GETINFO:
		return uput(&display[0], ptr, sizeof(struct display));
	case GFXIOC_MAP:
		return uput(&displaymap, ptr, sizeof(displaymap));
	case GFXIOC_UNMAP:
		return 0;
	}
	return -1;
}
