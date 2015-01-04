#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <vt.h>
#include <tty.h>

#undef  DEBUG			/* UNdefine to delete debug code sequences */

__sfr __at 0x2F tty_debug2;
__sfr __at 0xAA kbd_row_set;
__sfr __at 0xA9 kbd_row_read;

char tbuf1[TTYSIZ];
char tbuf2[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2}
};

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
bool tty_writeready(uint8_t minor)
{
	minor;
	return 1;
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


uint8_t keymap[11];
static uint8_t keyin[11];
static uint8_t keybyte, keybit;
static uint8_t newkey;
static int keysdown = 0;
static uint8_t shiftmask[11] = {
	0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0
};

static void keyproc(void)
{
	int i;
	uint8_t key;

	for (i = 0; i < 11; i++) {
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

/* TODO: use locale indicator in addr 002c BIOS, for now only
         international layout */

uint8_t keyboard[11][8] = {
	{'0','1','2', '3','4','5','6','7'},
	{'8','9','-','=','\\','[',']',';'},
	{ 0,  0, ',', '.','/',' ','a','b'},
	{'c','d','e', 'f','g','h','i','j'},
	{'k','l','m', 'n','o','p','q','r'},
	{'s','t','u', 'v','w','x','y','z'},
	{ 0 , 0 , 0 ,  0 , 0 , 0 , 0 , 0 }, /* f3 f2 f1 code caps graph ctrl shift */
	{ 0 , 0, 27 , '\t',24 ,8 , 0 , 13}, /* ret select bs stop tab esc f5 f4 */
	{32 ,12,  0 , 127, 0 , 0 , 0 , 0 }, /* right down up left del ins home space */
	{'*','+','/','0','1' ,'2','3','4'}, /* numeric keyboard */
	{'5','6','7','8','9' ,'-',',','.'}
};

uint8_t shiftkeyboard[11][8] = {
	{')','!','@', '#','$','%','^','&'},
	{'*','(','_','+','|','{','}',':'},
	{'"','~','<','>','?',' ','A','B'},
	{'C','D','E', 'F','G','H','I','J'},
	{'K','L','M', 'N','O','P','Q','R'},
	{'S','T','U', 'V','W','X','Y','Z'},
	{ 0 , 0 , 0 ,  0 , 0 , 0 , 0 , 0 }, /* f3 f2 f1 code caps graph ctrl shift */
	{ 0 , 0, 27 , '\t',24 ,8 , 0 , 13}, /* ret select bs stop tab esc f5 f4 */
	{32 ,12,  0 , 127, 0 , 0 , 0 , 0 }, /* right down up left del ins home space */
	{'*','+','/','0','1' ,'2','3','4'}, /* numeric keyboard */
	{'5','6','7','8','9' ,'-',',','.'}
};

static uint8_t capslock = 0;

static void keydecode(void)
{
	uint8_t c;

	if (keybyte == 6 && keybit == 3) {
		capslock = 1 - capslock;
		return;
	}

	if (keymap[6] & 1)	/* shift */
		c = shiftkeyboard[keybyte][keybit];
	else
		c = keyboard[keybyte][keybit];

	if (keymap[6] & 2) {	/* control */
		if (c > 31 && c < 96)
			c &= 31;
	}

	if (capslock && c >= 'a' && c <= 'z')
		c -= 'a' - 'A';

	/* TODO: function keys (F1-F10), graph, code */

	tty_inproc(1, c);
}

void update_keyboard()
{
	int n;
	uint8_t r;

	/* encode keyboard row in bits 0-3 0xAA, then read status from 0xA9 */
	for (n =0; n < 11; n++) {
		r = kbd_row_set & 0xf0 | n;
		kbd_row_set = r;
		keyin[n] = ~kbd_row_read;
	}
}


void kbd_interrupt(void)
{
	newkey = 0;
	update_keyboard();
	keyproc();

	if (keysdown < 3 && newkey)
		keydecode();
}

/* This is used by the vt asm code, but needs to live in the kernel */
uint16_t cursorpos;

