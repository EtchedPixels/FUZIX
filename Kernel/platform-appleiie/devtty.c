#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <device.h>
#include <vt.h>
#include <tty.h>

/* Ignore super serial and friends for the moment */

#define kbd_read ((volatile uint8_t *)0xC000)
#define kbd_strobe ((volatile uint8_t *)0xC010)
/* Assume a //c for the moment */
#define irq_check ((volatile uint8_t *)0xC041)
#define irq_reset ((volatile uint8_t *)0xC019)

static char tbuf1[TTYSIZ];
PTY_BUFFERS;

uint8_t vtattr_cap = 0;

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	PTY_QUEUES
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS	/* Nothing configurable */
};

/* tty1 is the screen tty2+ are the serial ports */

/* Output for the system console (kprintf etc) */
void kputchar(char c)
{
	if (c == '\n')
		tty_putc(1, '\r');
	tty_putc(1, c);
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
        return TTY_READY_NOW;
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	vtoutput(&c,1);
}

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
	minor;
}

void tty_sleeping(uint_fast8_t minor)
{
	minor;
}

int tty_carrier(uint_fast8_t minor)
{
	minor;
	return 1;
}

void tty_data_consumed(uint_fast8_t minor)
{
}

/* Beware - this kbd access also disables 80store */
void tty_poll(void)
{
        uint8_t x;
        
        x = *kbd_read;
        if (x & 0x80) {
		tty_inproc(1, x & 0x7F);
		x = *kbd_strobe;
	}
}

uint8_t check_timer(void)
{
	/* For now asume mouse card IIc - hack. Once we have proper IRQ
	   handling in place we can key this appropriately */
	if (*irq_check & 0x80) {
		*irq_reset;
		return 1;
	}
	return 0;
}

void plt_interrupt(void)
{
	tty_poll();
	if (check_timer())
		timer_interrupt();
}

/* Video driver: Some of this would be better in asm, especially the scrolling */

/* Line start table for 40 or 80 column. The only difference is that for
   40 column you add X, for 80 column you add X/2 and bit 0 of X tells you
   if its alternate (0) or main (1) memory. */
static uint8_t *vtmap[24] = {
	(uint8_t *)0x400,
	(uint8_t *)0x480,
	(uint8_t *)0x500,
	(uint8_t *)0x580,
	(uint8_t *)0x600,
	(uint8_t *)0x680,
	(uint8_t *)0x700,
	(uint8_t *)0x780,
	
	(uint8_t *)0x428,
	(uint8_t *)0x4A8,
	(uint8_t *)0x528,
	(uint8_t *)0x5A8,
	(uint8_t *)0x628,
	(uint8_t *)0x6A8,
	(uint8_t *)0x728,
	(uint8_t *)0x7A8,
	
	(uint8_t *)0x450,
	(uint8_t *)0x4D0,
	(uint8_t *)0x550,
	(uint8_t *)0x5D0,
	(uint8_t *)0x650,
	(uint8_t *)0x6D0,
	(uint8_t *)0x750,
	(uint8_t *)0x7D0
};

/* Simple driver for 40 column text */
/* 80 we just need to shift x right 1 and use the low bit as bank self */
void plot_char(int8_t y, int8_t x, uint16_t c)
{
	*(vtmap[y] + x) = ((uint8_t)c) | 128;
}

/* Point at ourselves so the first dummy cursor_off is harmless */
static uint8_t *cursorptr = (uint8_t *)&cursorptr;

void cursor_off(void)
{
	*cursorptr |= 128;
}

void cursor_on(int8_t y, int8_t x)
{
	cursorptr = vtmap[y] + x;
	*cursorptr &= 127;
}

/* Not a hardware cursor */
void cursor_disable(void)
{
}

void clear_across(int8_t y, int8_t x, int16_t l)
{
	uint8_t *addr = vtmap[y] + x;
	memset(addr, ' '|0x80, l);
}

void clear_lines(int8_t y, int8_t n)
{
	uint8_t *addr;
	while(n--) {
		addr = vtmap[y++];
		memset(addr, ' '|0x80, 40);
	}
}

void vtattr_notify(void)
{
}

void scroll_up(void)
{
	uint8_t y;
	for (y = 1; y <= 23; y++) {
		uint8_t *src = vtmap[y];
		uint8_t *dst = vtmap[y-1];
		memcpy(dst, src, 40);
	}
}

void scroll_down(void)
{
	uint8_t y;
	for (y = 23; y > 1; y--) {
		uint8_t *src = vtmap[y-1];
		uint8_t *dst = vtmap[y];
		memcpy(dst, src, 40);
	}
}

