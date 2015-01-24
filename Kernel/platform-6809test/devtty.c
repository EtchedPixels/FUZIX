#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <device.h>
#include <vt.h>
#include <tty.h>

#undef  DEBUG			/* UNdefine to delete debug code sequences */

uint8_t *uarta = (uint8_t *)0xFF04;
uint8_t *uartb = (uint8_t *)0xFF05;

char tbuf1[TTYSIZ];
char tbuf2[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2}
};

static void nap(void)
{
}

/* tty1 is the screen tty2 is the serial port */

/* Output for the system console (kprintf etc) */
void kputchar(char c)
{
	if (c == '\n')
		tty_putc(1, '\r');
	tty_putc(1, c);
}

void tty_putc(uint8_t minor, unsigned char c)
{
	minor;
#if 0
	if (minor == 1) {
		vtoutput(&c, 1);
		return;
	}
#endif	
	while(!(*uartb & 1));
	*uarta = c;
}

void tty_setup(uint8_t minor)
{
    minor;
}

/* For the moment */
int tty_carrier(uint8_t minor)
{
    minor;
    return 1;
}

void platform_interrupt(void)
{
	timer_interrupt();
}

/* This is used by the vt asm code, but needs to live at the top of the kernel */
uint16_t cursorpos;
