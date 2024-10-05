#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <vt.h>
#include <devtty.h>
#include <graphics.h>

static uint8_t tbuf1[TTYSIZ];

static uint8_t sleeping;

static uint8_t vtbuf[256];
static uint8_t *vtptr = vtbuf;
static uint8_t vidmode;

static const struct display mode[2] = {
	{
		0,
		64, 32,
		32, 16,
		255, 255,
		FMT_4PIXEL_128,
		HW_UNACCEL,
		GFX_MAPPABLE|GFX_MULTIMODE|GFX_TEXT
	}, {
		0,
		128, 96,
		0, 0,
		255, 255,
		FMT_COLOUR4,
		HW_UNACCEL,
		GFX_MAPPABLE|GFX_MULTIMODE
	}
};

struct videomap videomap = {
	0,
	0,
	0x7000,
	0x0800,
	0, 0,
	1,
	MAP_FBMEM_SIMPLE|MAP_FBMEM
};

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

static void vtflush(void)
{
	if (vtptr != vtbuf) {
		if (vidmode == 0)
			vtoutput(vtbuf, vtptr-vtbuf);
		vtptr = vtbuf;
	}
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	irqflags_t irq = di();
	/* Q: should we wait in this case or take the snow hit */
	if (vtptr == vtbuf + sizeof(vtbuf))
		vtflush();
	*vtptr++ = c;
	irqrestore(irq);
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
	vtflush();
	if (c == '\n')
		kputchar('\r');
	if (vidmode == 0)
		vtoutput(&c, 1);
	vtflush();
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

static uint8_t shiftmask[8] = { 0, 0x04, 0x04, 0, 0, 0, 0, 0 };

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
	/* TODO: can we check the whole keyboard with a single (6800) !+ 0xFF if
	   we know it was all up before ? */
	__asm
		ld hl,#_keybuf
		ld de, #0x68FE
		ld b, #8        ; 8 keyboard ports, 7FFE, BFFE, DFFE and so on
	read_halfrow:
		ld a, (de)
		cpl
		ld (hl), a
		rlc e
		inc hl
		djnz read_halfrow
	__endasm;
}

static uint8_t cursor[4] = { KEY_LEFT, KEY_DOWN, KEY_UP, KEY_RIGHT };

static void keydecode(void)
{
	uint8_t c;

	uint8_t ss = keymap[1] & 0x04;	/* SHIFT */
	uint8_t cs = keymap[2] & 0x04;	/* CTRL (we use for caps) */

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


void tty_pollirq(unsigned irq)
{
	int i;

	/* Try and do vt updates on the vblank to reduce snow */
	if (irq) {
		vtflush();
		wakeup(&vidmode);
	}

	/* If no key was down and all the keyyboard rows scanned at once
	   says everything is up then skip doing any work */
	if (keysdown == 0 && *((volatile uint8_t *)0x6800) == 0xFF)
		return;

	update_keyboard();

	newkey = 0;

	for (i = 0; i < 8; i++) {
		int n;
		uint8_t key = keybuf[i] ^ keymap[i];
		if (key) {
			uint8_t m = 0x20;
			for (n = 5; n >= 0; n--) {
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

int gfx_ioctl(uint8_t minor, uarg_t arg, char *ptr)
{
	uint8_t m;
	switch(arg) {
	case GFXIOC_GETINFO:
		return uput(&mode[vidmode], ptr, sizeof(struct display));
	case GFXIOC_MAP:
		return uput(&videomap, ptr, sizeof(struct videomap));
	case GFXIOC_GETMODE:
	case GFXIOC_SETMODE:
		m = ugetc(ptr);
		if (m > 1) {
			udata.u_error = EINVAL;
			return -1;
		}
		if (arg == GFXIOC_GETMODE)
			return uput(&mode[vidmode], ptr, sizeof(struct display));
		vidmode = m;
		vtflush();
		*((volatile uint8_t *)0x6800) = vidmode << 3;
		return 0;
	case GFXIOC_WAITVB:
		psleep(&vidmode);
		return 0;
	/* TODO select orange/buff v green */
	}
	return vt_ioctl(minor, arg, ptr);
}
