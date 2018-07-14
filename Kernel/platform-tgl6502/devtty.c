#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <device.h>
#include <vt.h>
#include <tty.h>

#undef  DEBUG			/* UNdefine to delete debug code sequences */

static volatile uint8_t *uart = (volatile uint8_t *)0xFF00;

static char tbuf1[TTYSIZ];
PTY_BUFFERS;

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	PTY_QUEUES
};

/* tty1 is the screen tty2 is the serial port */

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
	minor;
	uart[3] = c;
}

void tty_setup(uint8_t minor)
{
	minor;
}

void tty_sleeping(uint8_t minor)
{
	minor;
}

/* For the moment */
int tty_carrier(uint8_t minor)
{
	minor;
	return 1;
}

void tty_data_consumed(uint8_t minor)
{
}

void tty_poll(void)
{
        uint8_t x;
        
        x = uart[1] & 4;
        if (x) {
        	x = uart[2];
		tty_inproc(1, x);
	}
}
                
void platform_interrupt(void)
{
	tty_poll();
	timer_interrupt();
}

/* This is used by the vt asm code, but needs to live at the top of the kernel */
uint16_t cursorpos;
