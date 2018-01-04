#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <device.h>
#include <vt.h>
#include <tty.h>

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
	uart[0] = c;
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

void tty_setup(uint8_t minor)
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
                
void platform_interrupt(void)
{
	uint8_t t = *timer;
	tty_poll();
	while(t--) {
		timer_interrupt();
	}
}
