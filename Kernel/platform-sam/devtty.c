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
static uint8_t shiftmask[8] = {
    0, 0, 0, 0, 0, 0, 0, 7
};

static void keyproc(void)
{
	int i;
	uint8_t key;

	/* Call asm keyin here FIXME */
	for (i = 0; i < 8; i++) {
	        /* Set one of A0 to A7, and read the byte we get back.
	           Invert that to get a mask of pressed buttons */
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

static uint8_t keyboard[8][8] = {
	{'@', 'a', 'b', 'c', 'd', 'e', 'f', 'g' },
	{'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o' },
	{'p', 'q', 'r', 's', 't', 'u', 'v', 'w' },
	{'x', 'y', 'z', '[', '\\', ']', '^', '_' },
	{'0', '1', '2', '3', '4', '5', '6', '7' },
	{'8', '9', ':', ';', ',', '-', '.', '/' },
	{13, 12, 3, 0/*up*/, 0/*down*/, 8/* left */, 0/*right*/, ' '},
	{ 0, 0, 0, 0, 0xF1, 0xF2, 0xF3, 0 }
};

static uint8_t shiftkeyboard[8][10] = {
	{'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G' },
	{'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O' },
	{'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W' },
	{'X', 'Y', 'Z', '{', '|', '}', '^', '_' },
	{'0', '!', '"', '#', '$', '%', '&', '\'' },
	{'(', ')', '*', '+', '<', '=', '>', '?' },
	{13, 12, 3, 0/*up*/, 0/*down*/, 8/* left */, 0/*right*/, ' '},
	{ 0, 0, 0, 0, 0xF1, 0xF2, 0xF3, 0 }
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
                if (keymap[7] & 3) {
                    if (c == '(')
                        c = '{';
                    if (c == ')')
                        c = '}';
                    if (c == '-')
                        c = '_';
                    if (c == '/')
                        c = '``';
                    if (c == '<')
                        c = '^';
                } else {
                    if (c == '(')
                        c = '[';
                    if (c == ')')
                        c = ']';
                    if (c == '-')
                        c = '|';
                }
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

