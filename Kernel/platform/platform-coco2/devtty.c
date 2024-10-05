#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <device.h>
#include <devfd.h>
#include <vt.h>
#include <tty.h>
#include <graphics.h>

#undef  DEBUG			/* UNdefine to delete debug code sequences */

uint8_t *uart_data = (uint8_t *)0xFF04;	/* ACIA data */
uint8_t *uart_status = (uint8_t *)0xFF05;	/* ACIA status */
uint8_t *uart_command = (uint8_t *)0xFF06;	/* ACIA command */
uint8_t *uart_control = (uint8_t *)0xFF07;	/* ACIA control */

static unsigned char tbuf1[TTYSIZ];
static unsigned char tbuf2[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2}
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
	_CSYS|CBAUD|CSIZE|PARENB|PARODD|PARMRK
};

uint8_t vtattr_cap = 0;
struct vt_repeat keyrepeat;
static uint8_t kbd_timer;

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
	uint8_t c;
	if (minor == 1)
		return TTY_READY_NOW;
	c = *uart_status;
	return (c & 16) ? TTY_READY_NOW : TTY_READY_SOON; /* TX DATA empty */
}

/* For DragonPlus we should perhaps support both monitors 8) */

void tty_putc(uint8_t minor, unsigned char c)
{
	if (minor == 1)
		vtoutput(&c, 1);
	else
		*uart_data = c;	/* Data */
}

void tty_sleeping(uint8_t minor)
{
    used(minor);
}

void tty_setup(uint8_t minor, uint8_t flags)
{
	if (minor == 2) {
		/* FIXME: do proper mode setting */
		*uart_command = 0x01;	/* DTR high, IRQ enabled, TX irq disabled 8N1 */
		*uart_control = 0x1E;	/* 9600 baud */
	}
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

void tty_data_consumed(uint8_t minor)
{
}

uint8_t keymap[8];
static uint8_t keyin[8];
static uint8_t keybyte, keybit;
static uint8_t newkey;
static int keysdown = 0;
/* FIXME: shouldn't COCO shiftmask also differ ??? 0x02 not 0x40 ?? */
static uint8_t shiftmask[8] = {
	0, 0x40, 0, 0, 0, 0, 0, 0x40
};

static uint8_t rbit[8] = {
	0xFE,
	0xFD,
	0xFB,
	0xF7,
	0xEF,
	0xDF,
	0xBF,
	0x7F,
};

/* Row inputs: multiplexed with the joystick */
static volatile uint8_t *pia_row = (uint8_t *)0xFF00;
/* Columns for scanning: multiplexed with the printer port */
static volatile uint8_t *pia_col = (uint8_t *)0xFF02;
/* Control */
static volatile uint8_t *pia_ctrl = (uint8_t *)0xFF03;

static uint8_t keyany(void)
{
	uint8_t x;
	*pia_col = 0x00;
	x = ~*pia_row;
	*pia_col = 0xFF;
	return x;
}

static void keyproc(void)
{
	int i;
	uint8_t key;

	for (i = 0; i < 8; i++) {
		/* We do the scan in software on the Dragon */
		*pia_col = rbit[i];
		keyin[i] = ~*pia_row;
		key = keyin[i] ^ keymap[i];
		if (key) {
			int n;
			int m = 1;
			for (n = 0; n < 7; n++) {
				if ((key & m) && (keymap[i] & m)) {
					if (!(shiftmask[i] & m))
						keysdown--;
				}
				if ((key & m) && !(keymap[i] & m)) {
					if (!(shiftmask[i] & m)) {
						keysdown++;
						newkey = 1;
						keybyte = i;
						keybit = n;
					}
				}
				m += m;
			}
		}
		keymap[i] = keyin[i];
	}
	if (system_id) { 	/* COCO series */
	  keybit += 2;
	  if (keybit > 5)
	    keybit -= 6;
        }
}

uint8_t keyboard[8][7] = {
	{ '0', '8', '@', 'h', 'p', 'x', KEY_ENTER },
	{ '1', '9', 'a', 'i', 'q', 'y', 0 /* clear - used as ctrl*/ },
	{ '2', ':', 'b', 'j', 'r', 'z', KEY_ESC /* break (used for esc) */ },
	{ '3', ';', 'c', 'k', 's', '^' /* up */, 0 /* NC */ },
	{ '4', ',', 'd', 'l', 't', '|' /* down */, 0 /* NC */ },
	{ '5', '-', 'e', 'm', 'u', KEY_BS /* left */, 0 /* NC */ },
	{ '6', '.', 'f', 'n', 'v', KEY_TAB /* right */, 0 /* NC */ },
	{ '7', '/', 'g', 'o', 'w', ' ', 0 /* shift */ },
};

uint8_t shiftkeyboard[8][7] = {
	{ '_', '(', '\\', 'H', 'P', 'X', KEY_ENTER },
	{ '!', ')', 'A', 'I', 'Q', 'Y', 0 /* clear - used as ctrl*/ },
	{ '"', '*', 'B', 'J', 'R', 'Z', CTRL('C') /* break */ },
	{ '#', '+', 'C', 'K', 'S', '[' /* up */, 0 /* NC */ },
	{ '$', '<', 'D', 'L', 'T', ']' /* down */, 0 /* NC */ },
	{ '%', '=', 'E', 'M', 'U', '{' /* left */, 0 /* NC */ },
	{ '&', '>', 'F', 'N', 'V', '}' /* right */, 0 /* NC */ },
	{ '\'', '?', 'G', 'O', 'W', ' ', 0 /* shift */ },
};

static void keydecode(void)
{
	uint8_t c;

	if (keymap[7] & 64)	/* shift */
		c = shiftkeyboard[keybyte][keybit];
	else
		c = keyboard[keybyte][keybit];
	if (keymap[1] & 64) {	/* control */
		if (c > 31 && c < 127)
			c &= 31;
	}
	tty_inproc(1, c);
}

uint8_t sys_hz;

void plt_interrupt(void)
{
	static uint8_t tick;
	uint8_t i = *pia_ctrl;
	if (i & 0x80) {
		if (keysdown || keyany()) {
			*pia_col;
			newkey = 0;
			keyproc();
			if (keysdown && keysdown < 3) {
				if (newkey) {
					keydecode();
					kbd_timer = keyrepeat.first;
				} else if (! --kbd_timer) {
					keydecode();
					kbd_timer = keyrepeat.continual;
				}
			}
		}
		tick++;
		if (tick == sys_hz) {
			tick = 0;
			timer_interrupt();
		}
	}
}

void plt_reinterrupt(void)
{
	panic("reint");
}

/* This is used by the vt asm code, but needs to live at the top of the kernel */
uint16_t cursorpos;

