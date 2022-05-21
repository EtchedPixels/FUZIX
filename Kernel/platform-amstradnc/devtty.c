/*
 *	The NC100 has an 8251A clone in it. I'm not clear how complete a
 *	clone it is but as we probably don't want to run X.25 or BISYNC
 *	on the NC100 it shouldn't be an issue
 *
 *	TODO:
 *		RTS/CTS support
 *		Framing/parity error
 *		Break support
 */
#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <device.h>
#include <vt.h>
#include <tty.h>

#ifdef CONFIG_NC200
#include <devfd.h>
#endif

#undef  DEBUG			/* UNdefine to delete debug code sequences */

__sfr __at 0xC0 uarta;
__sfr __at 0xC1 uartb;

__sfr __at 0x90 irqmap;

__sfr __at 0xB0 kmap0;
__sfr __at 0xB1 kmap1;
__sfr __at 0xB2 kmap2;
__sfr __at 0xB3 kmap3;
__sfr __at 0xB4 kmap4;
__sfr __at 0xB5 kmap5;
__sfr __at 0xB6 kmap6;
__sfr __at 0xB7 kmap7;
__sfr __at 0xB8 kmap8;
__sfr __at 0xB9 kmap9;

uint8_t vtattr_cap;
struct vt_repeat keyrepeat;
static uint8_t kbd_timer;

static uint8_t txwait;

static char tbuf1[TTYSIZ];
static char tbuf2[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2}
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
/* FIXME: The hardware is actually wired with full support for RTS/CTS
   DTR so we should add RTS/CTS support eventually */
	_CSYS|CBAUD|CSIZE|PARENB|PARODD|CSTOPB,
};


static void nap(void)
{
}

/* tty1 is the screen tty2 is the serial port */

int nc100_tty_open(uint_fast8_t minor, uint16_t flag)
{
	int err;
	if (!minor)
		minor = udata.u_ptab->p_tty;

	err = tty_open(minor, flag);
	if (err)
		return err;
	if (minor == 2) {
		mod_control(0, 0x10);	/* turn on the line driver */
		nap();
		/* Set the baud rate and parameters */
		tty_setup(2, 0);
#ifdef CONFIG_NC200
		mod_irqen(0x0C, 0x00);    /* single Rx/Tx interrupt on NC200 */
#else
		mod_irqen(0x03, 0x00);    /* separate Rx/Tx interrupts on NC100 */
#endif
	}
	return (0);
}


int nc100_tty_close(uint_fast8_t minor)
{
	tty_close(minor);
	if (ttydata[minor].users)
		return 0;
	if (minor == 2) {
#ifdef CONFIG_NC200
		mod_irqen(0x00, 0x0C);    /* single Rx/Tx interrupt on NC200 */
#else
		mod_irqen(0x00, 0x03);    /* separate Rx/Tx interrupts on NC100 */
#endif
		mod_control(0x10, 0);	/* turn off the line driver */
	}
	return (0);
}

/* Output for the system console (kprintf etc) */
void kputchar(uint_fast8_t c)
{
	if (c == '\n') {
		tty_putc(1, '\r');
		tty_putc(2, '\r');
	}
	tty_putc(1, c);
	tty_putc(2, c);
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
	uint8_t c;
	if (minor == 1)
		return TTY_READY_NOW;
	c = uartb;
	return (c & 1) ? TTY_READY_NOW : TTY_READY_SOON;
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	minor;
	if (minor == 1) {
		vtoutput(&c, 1);
		return;
	}
	uarta = c;
}

void tty_data_consumed(uint_fast8_t minor)
{
	used(minor);
}

/* We save the value so that we can use it for future changes like flow
   control and break support */
static uint8_t uart_ctrl;

/* Called to set baud rate etc */
void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
	uint8_t b;

	used(flags);

	if (minor == 1)
		return;
	b = ttydata[2].termios.c_cflag & CBAUD;
	if (b < B150)
		b = B150;
	if (b > B19200)
		b = B19200;
	ttydata[2].termios.c_cflag &= ~CBAUD;
	ttydata[2].termios.c_cflag |= b;
	/* 0 = B150 .. 7 = B19200 in the same spacing and order as termios,
	   how convenient */
	mod_control(b - B150, 0x7);
	/* Now we need to think about the XPD 71051. Unfortunately the manual
	   is in Japanese. Fortunately it seems to be an 8251 clone */
	/* Reset it */
	uartb = 0x40;
	nap();
	/* Program it */
	uart_ctrl = 0x03 | ((ttydata[2].termios.c_cflag & CSIZE) >> 2);
	if (ttydata[2].termios.c_cflag & PARENB) {
		uart_ctrl |= 0x10;
		if (ttydata[2].termios.c_cflag & PARODD)
			uart_ctrl |= 0x20;
	}
	if (ttydata[2].termios.c_cflag & CSTOPB)
		uart_ctrl |= 0xC0;
	uartb = uart_ctrl;
	nap();
	uartb = 0x37;		/* RTS | RX enable | DTR | TX enable */
	nap();
}

/* The NC100 and NC200 don't have a carrier input on the connector */
int tty_carrier(uint_fast8_t minor)
{
    used(minor);
    return 1;
}

void tty_sleeping(uint_fast8_t minor)
{
    used(minor);
    txwait = 1;
}

#define SER_INIT	0x4F	/*  1 stop,no parity,8bit,16x */
#define SER_RXTX	0x37

void nc100_tty_init(void)
{
  /* Reset the 8251 */
  mod_control(0x00, 0x08);
  nap();
  mod_control(0x08, 0x00);
  nap();
  /* Force into a sane state: See 8251A documentation */
  uartb = 0;
  uartb = 0;
  uartb = 0;
  /* Now set it up */
  tty_setup(2, 0);
}

uint8_t keymap[10];
static uint8_t keyin[10];
static uint8_t keybyte, keybit;
static uint8_t newkey;
static int keysdown = 0;
static uint8_t shiftmask[10] = {
	3, 3, 2, 0, 0, 0, 0, 0x10, 0, 0
};

static void keyproc(void)
{
	int i;
	uint8_t key;

	for (i = 0; i < 10; i++) {
		key = keyin[i] ^ keymap[i];
		if (key) {
			int n;
			int m = 128;
			for (n = 0; n < 8; n++) {
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
				m >>= 1;
			}
		}
		keymap[i] = keyin[i];
	}
}

#ifdef CONFIG_NC200
uint8_t keyboard[10][8] = {
	{0, 0, 0, KEY_ENTER, KEY_LEFT ,'4', 0, 0},
	{'9', 0, 0, 0, ' ', KEY_ESC, 0,/*ctrl*/ 0/*func*/},
	{0, '6', 0, '5', KEY_TAB, '1', 0/*sym*/, 0/*capslock*/},
	{'d', 's', 0, 'e', 'w', 'q', '2', '3'},
	{'f', 'r', 0, 'a', 'x', 'z', '7', '8'},
	{'c', 'g', 'y', 't', 'v', 'b', 0, 0},
	{'n', 'h', '/', '#',  KEY_RIGHT , KEY_DEL, KEY_DOWN , '6'},
	{'k', 'm', 'u', 0, KEY_UP , '\\', '=', 0},
	{',', 'j', 'i', '\'', '[', ']', '-', 0},
	{'.', 'o', 'l', ';', 'p', KEY_BS, '0', 0}
};

uint8_t shiftkeyboard[10][8] = {
	{0, 0, 0, KEY_ENTER, KEY_LEFT , '$', 0, 0},
	{'(', 0, 0, 0, ' ', KEY_STOP, 0, 0},
	{0, '^', 0, '%', KEY_TAB, '!', 0, 0},
	{'D', 'S', 0, 'E', 'W', 'Q', '"', KEY_POUND },
	{'F', 'R', 0, 'A', 'X', 'Z', '&', '*'},
	{'C', 'G', 'Y', 'T', 'V', 'B', 0, 0},
	{'N', 'H', '?', '~', KEY_RIGHT , KEY_DEL, KEY_DOWN , '^'},
	{'K', 'M', 'U', 0, KEY_UP , '|', '+', 0},
	{'<', 'J', 'I', '@', '{', '}', '_', 0},
	{'>', 'O', 'L', ':', 'P', 8, ')', 0 }
};
#else
uint8_t keyboard[10][8] = {
	{0, 0, 0, KEY_ENTER, KEY_LEFT, 0, 0, 0},
	{0, '5', 0, 0, ' ', KEY_ESC, 0, 0},
	{0, 0, 0, 0, KEY_TAB, '1', 0, 0},
	{'d', 's', 0, 'e', 'w', 'q', '2', '3'},
	{'f', 'r', 0, 'a', 'x', 'z', 0, '4'},
	{'c', 'g', 'y', 't', 'v', 'b', 0, 0},
	{'n', 'h', '/', '#', KEY_RIGHT , KEY_DEL, KEY_DOWN , '6'},
	{'k', 'm', 'u', 0, KEY_UP , '\\', '7', '='},
	{',', 'j', 'i', '\'', '[', ']', '-', '8'},
	{'.', 'o', 'l', ';', 'p', KEY_BS, '9', '0'}
};

uint8_t shiftkeyboard[10][8] = {
	{0, 0, 0, KEY_ENTER, KEY_LEFT , 0, 0, 0},
	{0, '%', 0, 0, ' ', KEY_STOP, 0, 0},
	{0, 0, 0, 0, KEY_TAB, '!', 0, 0},
	{'D', 'S', 0, 'E', 'W', 'Q', '"', KEY_POUND },
	{'F', 'R', 0, 'A', 'X', 'Z', 0, '$'},
	{'C', 'G', 'Y', 'T', 'V', 'B', 0, 0},
	{'N', 'H', '?', '~', KEY_RIGHT , KEY_DEL, KEY_DOWN , '^'},
	{'K', 'M', 'U', 0, KEY_UP, '|', '&', '+'},
	{'<', 'J', 'I', '@', '{', '}', '_', '*'},
	{'>', 'O', 'L', ':', 'P', KEY_BS, '(', ')'}
};
#endif

static uint8_t capslock = 0;

static void keydecode(void)
{
	uint8_t c;

	if (keybyte == 2 && keybit == 7) {
		capslock = 1 - capslock;
		return;
	}

	if (keymap[0] & 3)	/* shift */
		c = shiftkeyboard[keybyte][keybit];
	else
		c = keyboard[keybyte][keybit];
	if (keymap[1] & 2) {	/* control */
		if (c > 31 && c < 127)
			c &= 31;
	}
	if (keymap[1] & 1) {	/* function: not yet used */
		;
	}
//    kprintf("char code %d\n", c);
	if (keymap[2] & 1) {	/* symbol */
		;
	}
	if (capslock && c >= 'a' && c <= 'z')
		c -= 'a' - 'A';
	if (keymap[7] & 0x10) {	/* menu: not yet used */
		;
	}
	tty_inproc(1, c);
}


#ifdef CONFIG_NC200

void plt_interrupt(void)
{
	uint8_t a = irqmap;
	uint8_t c;
	if (!(a & 4)) {
		/* FIXME: need to check uart itself to see wake cause */
		wakeup(&ttydata[2]);
		/* work around sdcc bug */
		c = uarta;
		tty_inproc(2, c);
		if (txwait && (uartb & 1)) {
			tty_outproc(2);
			txwait = 0;
		}
	}
	if (!(a & 8)) {
		keyin[0] = kmap0;
		keyin[1] = kmap1;
		keyin[2] = kmap2;
		keyin[3] = kmap3;
		keyin[4] = kmap4;
		keyin[5] = kmap5;
		keyin[6] = kmap6;
		keyin[7] = kmap7;
		keyin[8] = kmap8;
		keyin[9] = kmap9;	/* This resets the scan for 10mS on */

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
		timer_interrupt();
		devfd_spindown();
	}
	if (!(a & 16)) {
		/* FIXME: Power button */
		;
	}
	if (!(a & 32)) {
		/* FIXME: FDC */
		;
	}
	/* clear the mask */
	irqmap = a;
}


#else

void plt_interrupt(void)
{
	uint8_t a = irqmap;
	uint8_t c;
	if (!(a & 2))
		wakeup(&ttydata[2]);
	if (!(a & 1)) {
		/* FIXME: we should look for errors here one day */
		/* work around sdcc bug */
		c = uarta;
		tty_inproc(2, c);
	}
	if (!(a & 8)) {
		keyin[0] = kmap0;
		keyin[1] = kmap1;
		keyin[2] = kmap2;
		keyin[3] = kmap3;
		keyin[4] = kmap4;
		keyin[5] = kmap5;
		keyin[6] = kmap6;
		keyin[7] = kmap7;
		keyin[8] = kmap8;
		keyin[9] = kmap9;	/* This resets the scan for 10mS on */

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
		timer_interrupt();
	}
	/* clear the mask */
	irqmap = a;
}

#endif

/* This is used by the vt asm code, but needs to live at the top of the kernel */
uint16_t cursorpos;
