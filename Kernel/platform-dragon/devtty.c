#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <device.h>
#include <vt.h>
#include <tty.h>

#undef  DEBUG			/* UNdefine to delete debug code sequences */

uint8_t *uart_data = (uint8_t *)0xFF04;	/* ACIA data */
uint8_t *uart_status = (uint8_t *)0xFF05;	/* ACIA status */
uint8_t *uart_command = (uint8_t *)0xFF06;	/* ACIA command */
uint8_t *uart_control = (uint8_t *)0xFF07;	/* ACIA control */

char tbuf1[TTYSIZ];
char tbuf2[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2}
};

/* tty1 is the screen tty2 is the serial port */

/* Output for the system console (kprintf etc) */
void kputchar(char c)
{
	if (c == '\n')
		tty_putc(1, '\r');
	tty_putc(1, c);
}

bool tty_writeready(uint8_t minor)
{
	uint8_t c;
	if (minor == 1)
		return 1;
	c = *uart_status;
	return c & 16;	/* TX DATA empty */
}

void tty_putc(uint8_t minor, unsigned char c)
{
#if 0
	if (minor == 1) {
		vtoutput(&c, 1);
		return;
	}
#endif	
	*uart_data = c;	/* Data */
}

void tty_setup(uint8_t minor)
{
	/* FIXME: do proper mode setting */
	*uart_command = 0x01;	/* DTR high, IRQ enabled, TX irq disabled 8N1 */
	*uart_control = 0x1E;	/* 9600 baud */
}

int tty_carrier(uint8_t minor)
{
	/* The serial DCD is status bit 5 but not wired */
	return 1;
}

void tty_interrupt(void)
{
	uint8_t r = *uart_status;
	if (r & 0x8) {
		r = *uart_data;
		tty_inproc(2,r);
	}	
}

void platform_interrupt(void)
{
	timer_interrupt();
}

/* This is used by the vt asm code, but needs to live at the top of the kernel */
uint16_t cursorpos;
