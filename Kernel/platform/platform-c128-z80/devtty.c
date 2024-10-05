#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <vt.h>
#include <devtty.h>

static uint8_t tbuf1[TTYSIZ];

static uint8_t sleeping;

uint8_t vtattr_cap = 0;		/* TODO: colour */
struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
};

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
}

int tty_carrier(uint_fast8_t minor)
{
	return 1;
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	vtoutput(&c, 1);
}

void tty_sleeping(uint_fast8_t minor)
{
	sleeping |= (1 << minor);
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
	return TTY_READY_NOW;
}

void tty_data_consumed(uint_fast8_t minor)
{
	used(minor);
}

/* kernel writes to system console -- never sleep! */
void kputchar(uint_fast8_t c)
{
	if (c == '\n')
		tty_putc(TTYDEV - 512, '\r');
	tty_putc(TTYDEV - 512, c);
}

/* C128 Keyboard scan - 8x8 matrix */

struct vt_repeat keyrepeat;
static uint8_t kbd_timer;

/* Need to find a home for {} []_ | \ */

uint8_t keyboard[8][8] = {
	{  KEY_STOP, 'q', 0/*CBM*/, ' ', 2, 0/*ctrl*/, KEY_BS, '1' },
	{ '/', '~', '=', 0/*shift*/, KEY_HOME, ';', '*', KEY_POUND },
	{ ',','@',':','.','-','l','p','+' },
	{ 'n','o','k','m','0','j','i','9' },
	{ 'v','u','h','b','8','g','y','7' },
	{ 'x','t','f','c','6','d','r','5' },
	{ 0  ,'e','s','z','4','a','w','3' },	/* starts lshift */
	{ KEY_DOWN, KEY_F5, KEY_F3, KEY_F1, KEY_F7, KEY_RIGHT, KEY_ENTER, KEY_DEL }
};

uint8_t shiftkeyboard[8][8] = {
	{  KEY_STOP, 'q', 0/*CBM*/, ' ', 2, 0/*ctrl*/, KEY_BS, '!' },
	{ '?', '~', '=', 0/*shift*/, KEY_HOME, '}', '*', KEY_POUND },
	{ '<','@','{','>','-','l','p','+' },
	{ 'n','o','k','m','0','j','i',')' },
	{ 'v','u','h','b','(','g','y','\'' },
	{ 'x','t','f','c','&','d','r','%' },
	{ 0  ,'e','s','z','%','a','w','#' },	/* starts lshift */
	{ KEY_UP, KEY_F6, KEY_F4, KEY_F2, KEY_F8, KEY_LEFT, KEY_ENTER, KEY_DEL }
};

static uint8_t shiftmask[8] = {
	0x24,
	0x08,
	0x00,
	0x00,
	0x00,
	0x00,
	0x01,
	0x00
};

/* buffer for port scan procedure */
uint8_t keybuf[8];
/* Previous state */
uint8_t keymap[8];

static uint8_t keybyte, keybit;
static uint8_t newkey;
static int keysdown = 0;

/*
 *	Walk a 0 bit along CIA #1 port A, recording the port B values. This captures
 *	they keyboard. Joytick is via a different setup and needs adding
 */
static void update_keyboard(void)
{
	__asm
		ld hl,#_keybuf
		ld b,#0xdc
		ld de,#0x087F
		ld c,#0x00
	read_row:
		out (c),e
		inc c
		in a,(c)
		dec c		
		cpl
		ld (hl),a
		inc hl
		rrc e
		dec d
		jr nz, read_row
	__endasm;
}

static void keydecode(void)
{
	uint8_t c;

	uint8_t ss = keymap[1] & 0x08;	/* SHIFT */
	uint8_t cs = keymap[2] & 0x08;	/* CTRL (we use for caps) */

	c = keyboard[keybyte][keybit];
	/* ctrl-shift => ctrl */
	if (cs && ss) {
		/* Don't map the arrows, but map 'rubout'. The arrows include
		   symbols like ctrl-m we want to be able to use */
		if (c == ';')
			c = KEY_BS;
		else
			c &= 31;
	} else if (cs) {
		if (c >= 'a' && c <= 'z')
			c -= 'a' - 'A';
	} else if (ss)
		c = shiftkeyboard[keybyte][keybit];
	tty_inproc(1, c);
}


void handle_keys(void)
{
	int i;

	update_keyboard();

	newkey = 0;

	for (i = 0; i < 8; i++) {
		int n;
		uint8_t key = keybuf[i] ^ keymap[i];
		if (key) {
			uint8_t m = 0x80;
			for (n = 0; n < 8; n++) {
				if ((key & m) && (keymap[i] & m))
					if (!(shiftmask[i] & m))
						keysdown--;

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
		keymap[i] = keybuf[i];
	}
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

