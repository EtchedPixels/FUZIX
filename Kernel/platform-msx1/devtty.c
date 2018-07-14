#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <vt.h>
#include <tty.h>

#undef  DEBUG			/* UNdefine to delete debug code sequences */

__sfr __at 0x2F tty_debug2;

char tbuf1[TTYSIZ];
char tbuf2[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2}
};

uint8_t vtattr_cap = 0;		/* For now */

/* tty1 is the screen tty2 is the debug port */

/* Output for the system console (kprintf etc) */
void kputchar(char c)
{
	/* Debug port for bringup */
	if (c == '\n')
		tty_putc(2, '\r');
	tty_putc(2, c);
}

/* Both console and debug port are always ready */
ttyready_t tty_writeready(uint8_t minor)
{
	minor;
	return TTY_READY_NOW;
}

void tty_putc(uint8_t minor, unsigned char c)
{
	minor;
//
//	if (minor == 1) {
		vtoutput(&c, 1);
//		return;
//	}
	tty_debug2 = c;	
}

int tty_carrier(uint8_t minor)
{
	minor;
	return 1;
}

void tty_setup(uint8_t minor)
{
	minor;
}

void tty_sleeping(uint8_t minor)
{
	minor;
}

void tty_data_consumed(uint8_t minor)
{
}

#if 0
static uint8_t keymap[10];
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

			}
		}
		keymap[i] = keyin[i];
	}
}

uint8_t keyboard[10][8] = {
	{0, 0, 0, 10, '?' /*left */ , 0, 0, 0},
	{0, '5', 0, 0, ' ', 27, 0, 0},
	{0, 0, 0, 0, '\t', '1', 0, 0},
	{'d', 's', 0, 'e', 'w', 'q', '2', '3'},
	{'f', 'r', 0, 'a', 'x', 'z', 0, '4'},
	{'c', 'g', 'y', 't', 'v', 'b', 0, 0},
	{'n', 'h', '/', '#', '?' /*right */ , 127, '?' /*down */ , '6'},
	{'k', 'm', 'u', 0, '?' /*up */ , '\\', '7', '='},
	{',', 'j', 'i', '\'', '[', ']', '-', '8'},
	{'.', 'o', 'l', ';', 'p', 8, '9', '0'}
};

uint8_t shiftkeyboard[10][8] = {
	{0, 0, 0, 10, '?' /*left */ , 0, 0, 0},
	{0, '%', 0, 0, ' ', 3, 0, 0},
	{0, 0, 0, 0, '\t', '!', 0, 0},
	{'D', 'S', 0, 'E', 'W', 'Q', '"', '?' /* pound */ },
	{'F', 'R', 0, 'A', 'X', 'Z', 0, '$'},
	{'C', 'G', 'Y', 'T', 'V', 'B', 0, 0},
	{'N', 'H', '?', '~', '?' /*right */ , 127, '?' /*down */ , '^'},
	{'K', 'M', 'U', 0, '?' /*up */ , '|', '&', '+'},
	{'<', 'J', 'I', '@', '{', '}', '_', '*'},
	{'>', 'O', 'L', ':', 'P', 8, '(', ')'}
};

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

#endif

void tty_interrupt(void)
{
#if 0
	uint8_t a = irqmap;
	uint8_t c;
	if (!(a & 2))
		wakeup(&ttydata[2]);
	if (!(a & 1)) {
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
		if (keysdown < 3 && newkey)
			keydecode();
		timer_interrupt();
	}

	/* clear the mask */
	irqmap = a;
#endif	
}

/* This is used by the vt asm code, but needs to live in the kernel */
uint16_t cursorpos;

