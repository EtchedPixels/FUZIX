#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <vt.h>
#include <devtty.h>
#include <stdarg.h>

char tbuf1[TTYSIZ];
char tbuf2[TTYSIZ];

struct  s_queue  ttyinq[NUM_DEV_TTY+1] = {       /* ttyinq[0] is never used */
    {   NULL,    NULL,    NULL,    0,        0,       0    },
    {   tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf2,   tbuf2,   tbuf2,   TTYSIZ,   0,   TTYSIZ/2 }
};

/* Write to system console */
void kputchar(char c)
{
    if(c=='\n')
        tty_putc(1, '\r');
    tty_putc(1, c);
}

uint8_t tty_writeready(uint8_t minor)
{
    return TTY_READY_NOW;
}

void tty_putc(uint8_t minor, unsigned char c)
{
    if (minor == 1)
        vtoutput(&c, 1);
}

void tty_interrupt(void)
{
}

void tty_setup(uint8_t minor)
{
}

int tty_carrier(uint8_t minor)
{
    return 1;
}

void tty_sleeping(uint8_t minor)
{
}

void tty_data_consumed(uint8_t minor)
{
}

static uint8_t keymap[9];
uint8_t keyin[9];
static uint8_t keybyte, keybit;
static uint8_t newkey;
static int keysdown = 0;
static uint8_t shiftmask[9] = {
    0x80, 0, 0, 0, 0, 0, 0, 0x40, 0x80,
};

static void keyproc(void)
{
	int i;
	uint8_t key;

	keyscan();

	for (i = 0; i < 9; i++) {
		key = keyin[i] ^ keymap[i];
		if (key) {
			int n;
			int m = 1;
			for (n = 0; n < 8; n++) {
				if ((key & m) && (keymap[i] & m)) {
					if (!(shiftmask[i] & m))
						keysdown--;
				}
				if ((key & m) && !(keymap[i] & m)) {
					if (!(shiftmask[i] & m))
						keysdown++;
					keybyte = i;
					keybit = n;
					newkey = 1;
				}
				m += m;

			}
		}
		keymap[i] = keyin[i];
	}
}

static uint8_t keyboard[9][8] = {
	{'@', 'z', 'x', 'c', 'v', KEY_F1, KEY_F2, KEY_F3 },
	{'a', 's', 'd', 'f', 'g', KEY_F4, KEY_F5, KEY_F6 },
	{'q', 'w', 'e', 'r', 't', KEY_F7, KEY_F8, KEY_F9 },
	{'1', '2', '3', '4', '5', KEY_ESC, KEY_TAB, KEY_CAPSLOCK },
	{'0', '9', '8', '7', '6', 0, 0, KEY_BS },
	{'p', 'o', 'i', 'u', 'y', 0, 0, KEY_F10 },
	{KEY_ENTER, 'l', 'k', 'j', 'h', 0, 0, 0 },
	{' ', 0 /* CTRL */, 'm', 'n', 'b', 0, 0, KEY_INSERT },
	{0/* CTRL*/, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, 0, 0, 0 }
};

static uint8_t shiftkeyboard[9][8] = {
	{'@', 'Z', 'X', 'C', 'V', KEY_F1, KEY_F2, KEY_F3 },
	{'A', 'S', 'D', 'F', 'G', KEY_F4, KEY_F5, KEY_F6 },
	{'Q', 'W', 'E', 'R', 'T', KEY_F7, KEY_F8, KEY_F9 },
	{'1', '2', '3', '4', '5', KEY_ESC, KEY_TAB, KEY_CAPSLOCK },
	{'0', '9', '8', '7', '6', 0, 0, KEY_BS },
	{'P', 'O', 'I', 'U', 'Y', 0, 0, KEY_F10 },
	{KEY_ENTER, 'L', 'K', 'J', 'H', 0, 0, 0 },
	{' ', 0 /* CTRL */, 'M', 'N', 'B', 0, 0, KEY_INSERT },
	{0/* CTRL*/, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, 0, 0, 0 }
};


static uint8_t capslock = 0;

static void keydecode(void)
{
	uint8_t c;

	if (keybyte == 7 && keybit == 3) {
		capslock = 1 - capslock;
		return;
	}

	if (keymap[7] & 3)	/* shift */
		c = shiftkeyboard[keybyte][keybit];
	else
		c = keyboard[keybyte][keybit];

        /* The keyboard lacks some rather important symbols so remap them
           with control */
	if (keymap[7] & 4) {	/* control */
		if (c > 31 && c < 96)
			c &= 31;
	}
	if (capslock && c >= 'a' && c <= 'z')
		c -= 'a' - 'A';
	if (c)
		tty_inproc(1, c);
}

void kbd_interrupt(void)
{
	newkey = 0;
	keyproc();
	if (keysdown < 3 && newkey)
		keydecode();
}

