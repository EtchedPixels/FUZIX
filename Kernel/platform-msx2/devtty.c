#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <vt.h>
#include <tty.h>
#include "msx2.h"

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

/*
 * International key matrix
 */
uint8_t keyboard[11][8] = {
	{'0','1','2', '3','4','5','6','7'},
	{'8','9','-','=','\\','[',']',';'},
	{ 39, '`', ',', '.','/',' ','a','b'},
	{'c','d','e', 'f','g','h','i','j'},
	{'k','l','m', 'n','o','p','q','r'},
	{'s','t','u', 'v','w','x','y','z'},
	{0/*SHIFT*/,0/*CTRL*/,0/*GRPH*/,0/*CAPS*/,0/*CODE*/ , KEY_F3 , KEY_F2 , KEY_F1 },
	{KEY_F4 , KEY_F5,  KEY_ESC , '\t', KEY_STOP ,KEY_BS , 0 , 13},
	{32 , KEY_HOME,  KEY_INSERT , KEY_DEL, KEY_LEFT , KEY_UP , KEY_DOWN , KEY_RIGHT},
	{'*','+','/','0','1' ,'2','3','4'},
	{'5','6','7','8','9' ,'-',',','.'}
};

uint8_t shiftkeyboard[11][8] = {
	{')','!','@', '#','$','%','^','&'},
	{'*','(','_','+','|','{','}',':'},
	{'"','~','<','>','?',' ','A','B'},
	{'C','D','E', 'F','G','H','I','J'},
	{'K','L','M', 'N','O','P','Q','R'},
	{'S','T','U', 'V','W','X','Y','Z'},
	{0/*SHIFT*/,0/*CTRL*/,0/*GRPH*/,0/*CAPS*/,0/*CODE*/, KEY_F3 , KEY_F2 , KEY_F1 },
	{KEY_F4 , KEY_F5,  KEY_ESC , '\t', KEY_STOP ,KEY_BS , 0/*SELECT*/ , 13},
	{32 ,KEY_HOME,  KEY_INSERT , KEY_DEL, KEY_LEFT , KEY_UP , KEY_DOWN , KEY_RIGHT},
	{'*','+','/','0','1' ,'2','3','4'},
	{'5','6','7','8','9' ,'-',',','.'}
};

/*
 * Japan
 */
uint8_t keyboard_jp[3][8] = {
	{'0','1','2', '3','4','5','6','7'},
	{'8','9','-','^',KEY_YEN,'@','[',';'},
	{':',']', ',', '.','/',' ','a','b'}};

uint8_t shiftkeyboard_jp[3][8] = {
	{' ','!','"', '#','$','%','&',39},
	{'(',')','=','~','|','`','{','+'},
	{'*','}','<','>','?','_','A','B'}};
/*
 * UK
 */
uint8_t shiftkeyboard_uk[1][8] = {
	{39,'`', ',', '.','/',KEY_POUND,'A','B'}};  /* row 2 */

/*
 * Spanish
 */
uint8_t keyboard_es[2][8] = {
	{'8','9','-','=','\\','[',']', 'N'/* Ñ */}, /* row 1 */
	{39, ':', ',', '.','/',' ','a','b'}};

uint8_t shiftkeyboard_es[2][8] = {
	{'*','(','_','+','|','{','}', 'n' /* ñ */},  /* row 1 */
	{'"',':','<','>','?',' ','A','B'}};


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

	/* setup termios to use msx keys */
	ttydata[1].termios.c_cc[VERASE] = KEY_BS;
	ttydata[1].termios.c_cc[VSTOP] = KEY_STOP;
	ttydata[1].termios.c_cc[VSTART] = KEY_STOP;

	/* keyboard layout selection: default is international */
	if ((infobits & KBDTYPE_MASK) == KBDTYPE_JPN) {
	    memcpy(keyboard, keyboard_jp, 24);
	    memcpy(shiftkeyboard, shiftkeyboard_jp, 24);
	} else if ((infobits & KBDTYPE_MASK) == KBDTYPE_UK) {
	    memcpy(&shiftkeyboard[2][0],shiftkeyboard_uk,8);
	} else if ((infobits & KBDTYPE_MASK) == KBDTYPE_ES) {
	    memcpy(&keyboard[1][0], keyboard_es, 16);
	    memcpy(&shiftkeyboard[1][0], shiftkeyboard_es, 16);
	}
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

static uint8_t capslock = 0;

static void keydecode(void)
{
	uint8_t c;

	if (keybyte == 6 && keybit == 3) {
		capslock = 1 - capslock;
		return;
	}

	if (keymap[6] & 3 )	/* shift or control */
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

