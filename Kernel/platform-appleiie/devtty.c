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

/* tty1 is the screen tty2+ are the serial ports */

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
	vtoutput(&c,1);
}

void tty_setup(uint8_t minor)
{
	minor;
}

void tty_sleeping(uint8_t minor)
{
	minor;
}

int tty_carrier(uint8_t minor)
{
	minor;
	return 1;
}

void tty_data_consumed(uint8_t minor)
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

void platform_interrupt(void)
{
	tty_poll();
	if (check_timer())
		timer_interrupt();
}

/* Video driver: Some of this would be better in asm, especially the scrolling */

/* Line start table for 40 or 80 column. The only difference is that for
   40 column you add X, for 80 column you add X/2 and bit 0 of X tells you
   if its alternate (0) or main (1) memory. */
static volatile uint8_t *vtmap[24] = {
	(volatile uint8_t *)0x400,
	(volatile uint8_t *)0x480,
	(volatile uint8_t *)0x500,
	(volatile uint8_t *)0x580,
	(volatile uint8_t *)0x600,
	(volatile uint8_t *)0x680,
	(volatile uint8_t *)0x700,
	(volatile uint8_t *)0x780,
	
	(volatile uint8_t *)0x428,
	(volatile uint8_t *)0x4A8,
	(volatile uint8_t *)0x528,
	(volatile uint8_t *)0x5A8,
	(volatile uint8_t *)0x628,
	(volatile uint8_t *)0x6A8,
	(volatile uint8_t *)0x728,
	(volatile uint8_t *)0x7A8,
	
	(volatile uint8_t *)0x450,
	(volatile uint8_t *)0x4D0,
	(volatile uint8_t *)0x550,
	(volatile uint8_t *)0x5D0,
	(volatile uint8_t *)0x650,
	(volatile uint8_t *)0x6D0,
	(volatile uint8_t *)0x750,
	(volatile uint8_t *)0x7D0
};

/* Simple driver for 40 column text */

void plot_char(int8_t y, int8_t x, uint16_t c)
{
	*(vtmap[y] + x) = ((uint8_t)c) | 128;
}

/* Point at ourselves so the first dummy cursor_off is harmless */
static volatile uint8_t *cursorptr = (uint8_t *)&cursorptr;

void cursor_off(void)
{
	*cursorptr |= 128;
}

void cursor_on(int8_t y, int8_t x)
{
	cursorptr = vtmap[y] + x;
	*cursorptr &= 127;
}

void clear_across(int8_t y, int8_t x, int16_t l)
{
	volatile uint8_t *addr = vtmap[y] + x;
	memset(addr, ' '|0x80, l);
}

void clear_lines(int8_t y, int8_t n)
{
	volatile uint8_t *addr;
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
		volatile uint8_t *src = vtmap[y];
		volatile uint8_t *dst = vtmap[y-1];
		memcpy(dst, src, 40);
	}
}

void scroll_down(void)
{
	uint8_t y;
	for (y = 23; y > 1; y--) {
		volatile uint8_t *src = vtmap[y-1];
		volatile uint8_t *dst = vtmap[y];
		memcpy(dst, src, 40);
	}
}

