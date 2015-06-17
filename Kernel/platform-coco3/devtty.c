#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <device.h>
#include <vt.h>
#include <tty.h>
#include <devdw.h>
#include <ttydw.h>

#undef  DEBUG			/* UNdefine to delete debug code sequences */


uint8_t vtattr_cap;


char tbuf1[TTYSIZ];
char tbuf2[TTYSIZ];
char tbuf3[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf3, tbuf3, tbuf3, TTYSIZ, 0, TTYSIZ / 2}
};



static struct pty {
	unsigned char *base;	/* base of buffer in cpu space */
	unsigned char *cpos;	/* current location of cursor */
	unsigned char csave;	/* charactor that is under the cursor */
	struct vt_switch vt;	/* the vt.o module's state */
	unsigned int scrloc;	/* location to put into gimme */
};

static struct pty ptytab[] = {
	{(unsigned char *) 0xb400, NULL, 0, {0, 0, 0, 0}, 0xb400 / 8},
	{(unsigned char *) 0xac80, NULL, 0, {0, 0, 0, 0}, 0xac80 / 8}
};


/* ptr to current active pty table */
struct pty *curpty = &ptytab[0];

/* current minor for input */
int curminor = 1;


/* A wrapper for tty_close that closes the DW port properly */
int my_tty_close(uint8_t minor)
{
	if (minor == 3 && ttydata[3].users == 0)
		dw_vclose(minor);
	return (tty_close(minor));
}


/* Output for the system console (kprintf etc) */
void kputchar(char c)
{
	if (c == '\n')
		tty_putc(1, '\r');
	tty_putc(1, c);
}

ttyready_t tty_writeready(uint8_t minor)
{
	return TTY_READY_NOW;
}

void tty_putc(uint8_t minor, unsigned char c)
{
	if (minor == 3) {
		dw_putc(minor, c);
		return;
	}
	struct pty *t = curpty;
	vt_save(&curpty->vt);
	curpty = &ptytab[minor - 1];
	vt_load(&curpty->vt);
	vtoutput(&c, 1);
	vt_save(&curpty->vt);
	curpty = t;
	vt_load(&curpty->vt);
}

void tty_sleeping(uint8_t minor)
{
	used(minor);
}


void tty_setup(uint8_t minor)
{
	if (minor == 3) {
		dw_vopen(minor);
		return;
	}
}

/* For the moment */
int tty_carrier(uint8_t minor)
{
	return 1;
}

void tty_interrupt(void)
{

}

uint8_t keymap[8];
static uint8_t keyin[8];
static uint8_t keybyte, keybit;
static uint8_t newkey;
static int keysdown = 0;
static uint8_t shiftmask[8] = {
	0, 0, 0, 0x40, 0x40, 0, 0, 0x40
};

/* a lookup table to rotate a 0 bit around */
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
static volatile uint8_t *pia_row = (uint8_t *) 0xFF00;
/* Columns for scanning: multiplexed with the printer port */
static volatile uint8_t *pia_col = (uint8_t *) 0xFF02;


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
}

#ifdef CONFIG_COCO_KBD
uint8_t keyboard[8][7] = {
	{'@', 'h', 'p', 'x', '0', '8', KEY_ENTER}
	,
	{'a', 'i', 'q', 'y', '1', '9', 0 /* clear - used as ctrl */ }
	,
	{'b', 'j', 'r', 'z', '2', ':', KEY_ESC /* break (used for esc) */ }
	,
	{'c', 'k', 's', '^' /* up */ , '3', ';', 0 /* NC */ }
	,
	{'d', 'l', 't', '|' /* down */ , '4', ',', 0 /* NC */ }
	,
	{'e', 'm', 'u', KEY_BS /* left */ , '5', '-', '~' /* NC */ }
	,
	{'f', 'n', 'v', KEY_TAB /* right */ , '6', '.', 0 /* NC */ }
	,
	{'g', 'o', 'w', ' ', '7', '/', 0 /* shift */ }
	,
};

uint8_t shiftkeyboard[8][7] = {
	{'\\', 'H', 'P', 'X', '_', '(', KEY_ENTER}
	,
	{'A', 'I', 'Q', 'Y', '!', ')', 0 /* clear - used as ctrl */ }
	,
	{'B', 'J', 'R', 'Z', '"', '*', CTRL('C') /* break */ }
	,
	{'C', 'K', 'S', '[' /* up */ , '#', '+', 0 /* NC */ }
	,
	{'D', 'L', 'T', ']' /* down */ , '$', '<', 0 /* NC */ }
	,
	{'E', 'M', 'U', '{' /* left */ , '%', '=', '|' /* NC */ }
	,
	{'F', 'N', 'V', '}' /* right */ , '&', '>', 0 /* NC */ }
	,
	{'G', 'O', 'W', ' ', '\'', '?', 0 /* shift */ }
	,
};
#else
uint8_t keyboard[8][7] = {
	{'0', '8', '@', 'h', 'p', 'x', KEY_ENTER}
	,
	{'1', '9', 'a', 'i', 'q', 'y', 0 /* clear - used as ctrl */ }
	,
	{'2', ':', 'b', 'j', 'r', 'z', KEY_ESC /* break (used for esc) */ }
	,
	{'3', ';', 'c', 'k', 's', '^' /* up */ , 0 /* NC */ }
	,
	{'4', ',', 'd', 'l', 't', '|' /* down */ , 0 /* NC */ }
	,
	{'5', '-', 'e', 'm', 'u', KEY_BS /* left */ , 0 /* NC */ }
	,
	{'6', '.', 'f', 'n', 'v', KEY_TAB /* right */ , 0 /* NC */ }
	,
	{'7', '/', 'g', 'o', 'w', ' ', 0 /* shift */ }
	,
};

uint8_t shiftkeyboard[8][7] = {
	{'_', '(', '\\', 'H', 'P', 'X', KEY_ENTER}
	,
	{'!', ')', 'A', 'I', 'Q', 'Y', 0 /* clear - used as ctrl */ }
	,
	{'"', '*', 'B', 'J', 'R', 'Z', CTRL('C') /* break */ }
	,
	{'#', '+', 'C', 'K', 'S', '[' /* up */ , 0 /* NC */ }
	,
	{'$', '<', 'D', 'L', 'T', ']' /* down */ , 0 /* NC */ }
	,
	{'%', '=', 'E', 'M', 'U', '{' /* left */ , 0 /* NC */ }
	,
	{'&', '>', 'F', 'N', 'V', '}' /* right */ , 0 /* NC */ }
	,
	{'\'', '?', 'G', 'O', 'W', ' ', 0 /* shift */ }
	,
};
#endif				/* COCO_KBD */



static void keydecode(void)
{
	uint8_t c;

	/* shift shifted handling - use alt lookup table */
	if (keymap[7] & 64)
		c = shiftkeyboard[keybyte][keybit];
	else
		c = keyboard[keybyte][keybit];
	/* control shifted handling - we need some refactoring here. */
	if (keymap[4] & 64) {
		/* control+1 */
		if (c == '1') {
			vt_save(&curpty->vt);
			curpty = &ptytab[0];
			*(unsigned int *) 0xff9d = curpty->scrloc;
			vt_load(&curpty->vt);
			curminor = 1;
			return;
		}
		/* control + 2 */
		if (c == '2') {
			vt_save(&curpty->vt);
			curpty = &ptytab[1];
			*(unsigned int *) 0xff9d = curpty->scrloc;
			vt_load(&curpty->vt);
			curminor = 2;
			return;
		}
		/* control + something else */
		if (c > 32 && c < 127)
			c &= 31;
	}
	tty_inproc(curminor, c);
}

void platform_interrupt(void)
{
	*pia_col;
	newkey = 0;
	keyproc();
	if (keysdown < 3 && newkey)
		keydecode();
	timer_interrupt();
	dw_vpoll();
}




/* These are routines stolen from the stock vt.c's VT_SIMPLE code, and modified
   to suite multiple vts
*/


static uint8_t *char_addr(unsigned int y1, unsigned char x1)
{
	return curpty->base + VT_WIDTH * y1 + (uint16_t) x1;
}

void cursor_off(void)
{
	if (curpty->cpos)
		*curpty->cpos = curpty->csave;
}

void cursor_on(int8_t y, int8_t x)
{
	curpty->csave = *char_addr(y, x);
	curpty->cpos = char_addr(y, x);
	*curpty->cpos = VT_MAP_CHAR('_');
}

void plot_char(int8_t y, int8_t x, uint16_t c)
{
	*char_addr(y, x) = VT_MAP_CHAR(c);
}

void clear_lines(int8_t y, int8_t ct)
{
	unsigned char *s = char_addr(y, 0);
	memset(s, ' ', ct * VT_WIDTH);
}

void clear_across(int8_t y, int8_t x, int16_t l)
{
	unsigned char *s = char_addr(y, x);
	memset(s, ' ', l);
}

/* FIXME: these should use memmove */

void scroll_up(void)
{
	memcpy(curpty->base, curpty->base + VT_WIDTH,
	       VT_WIDTH * VT_BOTTOM);
}

void scroll_down(void)
{
	memcpy(curpty->base + VT_WIDTH, curpty->base,
	       VT_WIDTH * VT_BOTTOM);
}


unsigned char vt_map(unsigned char c)
{
	/* The CoCo3's gime has a strange code for underscore */
	if (c == '_')
		return 0x7F;
	return c;
}
