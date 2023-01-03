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

uint8_t keyboard[8][6] = {
	{ 't','w',0,'e','q','r' },
	{ 'g','s',/*ctrl*/0, 'd', 'a', 'f'},
	{ 'b','x',/*shift*/0,'c','z','v'},
	{ '5','2',0,'3','1','4' },
	{ 'n','.',0,',',' ','m' },
	{ '6','9','-','8','0','7' },
	{ 'y','o', KEY_ENTER,'i','p','u' },
	{ 'h','l',':','k',';','j' }
};

static uint8_t shiftmask[8] = { 0, 0, 0x08, 0x08, 0, 0, 0, 0 };

/* The VZ200 lacks {} | ~ so we map these onto UIJY */
uint8_t shiftkeyboard[8][6] = {
	{ 't','w',0,'e','q','r' },
	{ 'g','s',/*ctrl*/0, 'd', 'a', 'f'},
	{ 'b','x',/*shift*/0,'c','z','v'},
	{ '%','"',0,'#','!','$' },
	{ '^','>',0,'<',' ','\\' },
	{ '&',')','=','(','@','\'' },
	{ '~','[',KEY_ENTER,'}',']','{' },
	{ 'h','?','*','/','+','|' }
};

struct vt_repeat keyrepeat;
static uint8_t kbd_timer;

/* buffer for port scan procedure */
uint8_t keybuf[8];
/* Previous state */
uint8_t keymap[8];

static uint8_t keybyte, keybit;
static uint8_t newkey;
static int keysdown = 0;


/* Other problems: we have a control key, and a shift but shift is needed
   for the symbols, so we use ctrl as caps and ctrl-shift as ctrl */

static void update_keyboard(void)
{
	__asm
		ld hl,#_keybuf
		ld bc, #0x68FE
		ld e, #8        ; 8 keyboard ports, 7FFE, BFFE, DFFE and so on
	read_halfrow:
		in a, (c)
		cpl
		ld (hl), a
		rlc b
		inc hl
		dec e
		jr nz, read_halfrow
	__endasm;
}

void tty_pollirq(void)
{
	int i;

	update_keyboard();

	newkey = 0;

	for (i = 0; i < 8; i++) {
		int n;
		uint8_t key = keybuf[i] ^ keymap[i];
		if (key) {
			uint8_t m = 0x20;
			for (n = 4; n >= 0; n--) {
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

static uint8_t cursor[4] = { KEY_LEFT, KEY_DOWN, KEY_UP, KEY_RIGHT };

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
