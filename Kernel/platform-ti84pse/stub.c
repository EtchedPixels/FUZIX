/* TEMP */
#include <kernel.h>
#include <tty.h>
#include <stdbool.h>
#include <display.h>

/* TODO: Figure out TTY shit */
static char tbuf1[TTYSIZ];
static char tbuf2[TTYSIZ];
static char tbuf3[TTYSIZ];
static char tbuf4[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY+1] = { /* ttyinq[0] is never used */
	{   NULL,    NULL,    NULL,    0,        0,       0    },
	{   tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
	{   tbuf2,   tbuf2,   tbuf2,   TTYSIZ,   0,   TTYSIZ/2 },
	{   tbuf3,   tbuf3,   tbuf3,   TTYSIZ,   0,   TTYSIZ/2 },
	{   tbuf4,   tbuf4,   tbuf4,   TTYSIZ,   0,   TTYSIZ/2 }
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
	_CSYS,
	_CSYS,
	_CSYS
};

static int row, col;

static void newline()
{
	col = 0;
	if (++row >= 10) {
		/* TODO: Scrolling */
		row = 0;
		__asm
			halt
		__endasm;
	}
}

void kputchar(char c)
{
	switch (c) {
	case '\r':
		col = 0;
		return;
	case '\n':
		newline();
		return;
	case '\t':
		col += 4;
		if (col >= 24) {
			newline();
		}
		return;
	case '\0':
		return;
	}
	plot_char(row, col, c);
	copy_display();
	if (++col >= 24) {
		newline();
	}
}

void tty_setup(uint8_t minor, uint8_t flags)
{
	init_display();
	(void)minor;
	(void)flags;
}

int tty_carrier(uint8_t minor)
{
	(void)minor;
	return 1;
}

char tty_writeready(uint8_t minor)
{
	(void)minor;
	return 0;
}

void tty_putc(uint8_t minor, unsigned char c)
{
	(void)minor;
	(void)c;
}

void tty_sleeping(uint8_t minor)
{
	(void)minor;
}

void tty_data_consumed(uint8_t minor)
{
	(void)minor;
}
