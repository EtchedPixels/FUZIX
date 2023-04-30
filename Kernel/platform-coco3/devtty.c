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
#include <graphics.h>
#include <video.h>

#undef  DEBUG			/* UNdefine to delete debug code sequences */

#define VSECT	__attribute__((section(".video")))
#define VSECTD	__attribute__((section(".videodata")))


/* Default key repeat values, in tenths of seconds */
#define REPEAT_FIRST 5		/* delay before first repeat */
#define REPEAT_CONTINUAL 1	/* delay on sucessive repeats */


extern uint8_t hz;


uint8_t vtattr_cap;
uint8_t curattr;

#define tbuf1 (uint8_t *)(0x2000+TTYSIZ*0)
#define tbuf2 (uint8_t *)(0x2000+TTYSIZ*1)
#define tbuf3 (uint8_t *)(0x2000+TTYSIZ*2)
#define tbuf4 (uint8_t *)(0x2000+TTYSIZ*3)
#define tbuf5 (uint8_t *)(0x2000+TTYSIZ*4)
#define tbuf6 (uint8_t *)(0x2000+TTYSIZ*5)
#define tbuf7 (uint8_t *)(0x2000+TTYSIZ*6)
#define tbuf8 (uint8_t *)(0x2000+TTYSIZ*7)
#define tbuf9 (uint8_t *)(0x2000+TTYSIZ*8)
#define tbufa (uint8_t *)(0x2000+TTYSIZ*9)


struct s_queue ttyinq[NUM_DEV_TTY + 1] = {
	/* ttyinq[0] is never used */
	{ NULL, NULL, NULL, 0, 0, 0 },
	/* GIME Consoles */
	{ tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2 },
	{ tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2 },
	/* Drivewire Virtual Serial Ports */
	{ tbuf3, tbuf3, tbuf3, TTYSIZ, 0, TTYSIZ / 2 },
	{ tbuf4, tbuf4, tbuf4, TTYSIZ, 0, TTYSIZ / 2 },
	{ tbuf5, tbuf5, tbuf5, TTYSIZ, 0, TTYSIZ / 2 },
	{ tbuf6, tbuf6, tbuf6, TTYSIZ, 0, TTYSIZ / 2 },
	/* Drivewire Virtual Window Ports */
	{ tbuf7, tbuf7, tbuf7, TTYSIZ, 0, TTYSIZ / 2 },
	{ tbuf8, tbuf8, tbuf8, TTYSIZ, 0, TTYSIZ / 2 },
	{ tbuf9, tbuf9, tbuf9, TTYSIZ, 0, TTYSIZ / 2 },
	{ tbufa, tbufa, tbufa, TTYSIZ, 0, TTYSIZ / 2 },
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	/* GIME consoles */
	_CSYS,
	_CSYS,
	/* Drivewire */
	_CSYS,
	_CSYS,
	_CSYS,
	_CSYS,
	/* Virtual Window */
	_CSYS,
	_CSYS,
	_CSYS,
	_CSYS,
};

struct mode_s {
	uint8_t vmod;
	uint8_t vres;
	uint8_t width;
	uint8_t height;
	uint8_t right;
	uint8_t bottom;
	struct display *fmod;
};

/* List (array) of all supported modes, as relayed to ioctl */
static struct display fmodes[] = {
	{
	 0,			/* Mode  number */
	 80, 25,		/* screen size */
	 80, 25,		/* buffer size */
	 0xFF, 0xFF,		/* no pan, scroll */
	 FMT_TEXT,		/* this is a text mode */
	 HW_UNACCEL,		/* no acceleration */
	 GFX_PALETTE | GFX_MULTIMODE | GFX_PALETTE_SET | GFX_TEXT,	/* all the crap we support in this mode */
	 0,			/* Memory size irrelevant */
	 0,			/* supports no graphics commands */
	}, {
	 1,			/* Mode  number */
	 40, 25,		/* screen size */
	 40, 25,		/* buffer size */
	 0xFF, 0xFF,		/* no pan, scroll */
	 FMT_TEXT,		/* this is a text mode */
	 HW_UNACCEL,		/* no acceleration */
	 GFX_PALETTE | GFX_MULTIMODE | GFX_PALETTE_SET | GFX_TEXT,	/* all the crap we support in this mode */
	 0,			/* Memory size irrelevant */
	 0,			/* supports no graphics commands */
	}, {
	 2,			/* Mode  number */
	 64, 25,		/* screen size */
	 64, 25,		/* buffer size */
	 0xFF, 0xFF,		/* no pan, scroll */
	 FMT_TEXT,		/* this is a text mode */
	 HW_UNACCEL,		/* no acceleration */
	 GFX_PALETTE | GFX_MULTIMODE | GFX_PALETTE_SET | GFX_TEXT,	/* all the crap we support in this mode */
	 0,			/* Memory size irrelevant */
	 0,			/* supports no graphics commands */
	}, {
	 3,			/* Mode  number */
	 32, 25,		/* screen size */
	 32, 25,		/* buffer size */
	 0xFF, 0xFF,		/* no pan, scroll */
	 FMT_TEXT,		/* this is a text mode */
	 HW_UNACCEL,		/* no acceleration */
	 GFX_PALETTE | GFX_MULTIMODE | GFX_PALETTE_SET | GFX_TEXT,	/* all the crap we support in this mode */
	 0,			/* Memory size irrelevant */
	 0,			/* supports no graphics commands */
	}, {
	 4,			/* Mode  number */
	 256, 192,		/* screen size */
	 256, 192,		/* buffer size */
	 0xFF, 0xFF,		/* no pan, scroll */
	 FMT_MONO_WB,		/* for now just B&W */
	 HW_UNACCEL,		/* no acceleration */
	 0,			/* no features */
	 0,			/* Memory size irrelevant */
	 GFX_DRAW | GFX_READ | GFX_WRITE	/* only the basics */
	}
};

static struct mode_s mode[5] = {
	{ 0x04, 0x75, 80, 25, 79, 24, &(fmodes[0]) },
	{ 0x04, 0x6d, 40, 25, 39, 24, &(fmodes[1]) },
	{ 0x04, 0x71, 64, 25, 63, 24, &(fmodes[2]) },
	{ 0x04, 0x69, 32, 25, 31, 24, &(fmodes[3]) },
	{ 0x80, 0x08, 40, 21, 39, 20, &(fmodes[4]) },
};


static struct tty_coco3 ttytab[] VSECTD = {
	{
	 (unsigned char *) 0x2000,
	 NULL,
	 0,
	 { 0, 0, 0, 0, 0, 0, 7, 0},
	 0x10000 / 8,
	 0x04,
	 0x74,			/* 80 column */
	 80,
	 25,
	 79,
	 24,
	 &fmodes[0],
	}, {
	 (unsigned char *) 0x3000,
	 NULL,
	 0,
	 { 0, 0, 0, 0, 0, 0, 7, 0},
	 0x11000 / 8,
	 0x04,
	 0x6c,			/* 40 column */
	 40,
	 25,
	 39,
	 24,
	 &fmodes[1],
	}
};


/* ptr to current active tty table */
struct tty_coco3 *curtty VSECTD = &ttytab[0];

/* current minor for input */
int curminor = 1;

/* Apply settings to GIME chip */
void apply_gime(int minor)
{
	struct tty_coco3 *p = &(ttytab[minor - 1]);
	uint16_t s;
	if (p->vmod & 0x80)
		s = 0x12000 / 8;
	else
		s = p->scrloc;
	*(volatile uint16_t *) 0xff9d = s;
	*(volatile uint8_t *) 0xff98 = (hz & 0x78) | p->vmod;
	*(volatile uint8_t *) 0xff99 = p->vres;
}

/* A wrapper for tty_close that closes the DW port properly */
int my_tty_close(uint_fast8_t minor)
{
	if (minor > 2 && ttydata[minor].users == 1)
		dw_vclose(minor);
	return (tty_close(minor));
}

/* Output for the system console (kprintf etc) */
void kputchar(uint_fast8_t c)
{
	if (c == '\n')
		tty_putc(1, '\r');
	tty_putc(1, c);
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
	return TTY_READY_NOW;
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	int irq;
	if (minor > 2) {
		dw_putc(minor, c);
		return;
	}
	irq = di();
	if (curtty != &ttytab[minor - 1]) {
		vt_save(&curtty->vt);
		curtty = &ttytab[minor - 1];
		vt_load(&curtty->vt);
	}
	vtoutput(&c, 1);
	irqrestore(irq);
}

void tty_sleeping(uint_fast8_t minor)
{
	used(minor);
}

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
	if (minor > 2) {
		dw_vopen(minor);
		return;
	}
}

int tty_carrier(uint_fast8_t minor)
{
	if (minor > 2)
		return dw_carrier(minor);
	return 1;
}

void tty_interrupt(void)
{

}

void tty_data_consumed(uint_fast8_t minor)
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

struct vt_repeat keyrepeat;
static uint8_t kbd_timer;

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

uint8_t keyboard[8][7] = {
	{ '@', 'h', 'p', 'x', '0', '8', KEY_ENTER },
	{ 'a', 'i', 'q', 'y', '1', '9', 0 /* clear - used as ctrl */  },
	{ 'b', 'j', 'r', 'z', '2', ':', KEY_ESC /* break (used for esc) */  },
	{ 'c', 'k', 's', '^' /* up */ , '3', ';', 0 /* NC */  },
	{ 'd', 'l', 't', '|' /* down */ , '4', ',', 0 /* NC */  },
	{ 'e', 'm', 'u', KEY_BS /* left */ , '5', '-', '~' /* F1 */  },
	{ 'f', 'n', 'v', KEY_TAB /* right */ , '6', '.', '`' /* F2 */  },
	{ 'g', 'o', 'w', ' ', '7', '/', 0 /* shift */  }
};

uint8_t shiftkeyboard[8][7] = {
	{ '\\', 'H', 'P', 'X', '_', '(', KEY_ENTER },
	{ 'A', 'I', 'Q', 'Y', '!', ')', 0 /* clear - used as ctrl */  },
	{ 'B', 'J', 'R', 'Z', '"', '*', CTRL('C') /* break */  },
	{ 'C', 'K', 'S', '[' /* up */ , '#', '+', 0 /* NC */  },
	{ 'D', 'L', 'T', ']' /* down */ , '$', '<', 0 /* NC */  },
	{ 'E', 'M', 'U', '{' /* left */ , '%', '=', '|' /* F1 */  },
	{ 'F', 'N', 'V', '}' /* right */ , '&', '>', 0 /* F2 */  },
	{ 'G', 'O', 'W', ' ', '\'', '?', 0 /* shift */  },
};

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
			vt_save(&curtty->vt);
			curtty = &ttytab[0];
			vt_load(&curtty->vt);
			curminor = 1;
			apply_gime(1);
			return;
		}
		/* control + 2 */
		if (c == '2') {
			vt_save(&curtty->vt);
			curtty = &ttytab[1];
			vt_load(&curtty->vt);
			curminor = 2;
			apply_gime(2);
			return;
		}
		/* control + something else */
		if (c > 32 && c < 127)
			c &= 31;
	}
	tty_inproc(curminor, c);
}

void plt_interrupt(void)
{
	*pia_col;
	newkey = 0;
	keyproc();
	if (keysdown && (keysdown < 3)) {
		if (newkey) {
			keydecode();
			kbd_timer = keyrepeat.first;
		} else {
			if (!--kbd_timer) {
				keydecode();
				kbd_timer = keyrepeat.continual;
			}
		}
	}
	timer_interrupt();
	dw_vpoll();
}

/* At this point the COCO2 doesn't use interrupt nesting */
void plt_reinterrupt(void)
{
	panic("reint");
}

void vtattr_notify(void)
{
	curattr = ((vtink & 7) << 3) + (vtpaper & 7);
}

int gfx_ioctl(uint_fast8_t minor, uarg_t arg, char *ptr)
{
	if (minor > 2)		/* remove once DW get its own ioctl() */
		return tty_ioctl(minor, arg, ptr);
	if (arg >> 8 != 0x03)
		return vt_ioctl(minor, arg, ptr);
	switch (arg) {
	case GFXIOC_GETINFO:
		return uput(ttytab[minor - 1].fdisp, ptr, sizeof(struct display));
	case GFXIOC_GETMODE:
		{
			uint8_t m = ugetc(ptr);
			if (m > 4)
				goto inval;
			return uput(&fmodes[m], ptr, sizeof(struct display));
		}
	case GFXIOC_SETMODE:
		{
			uint8_t m = ugetc(ptr);
			if (m > 4)
				goto inval;
			memcpy(&(ttytab[minor - 1].vmod), &(mode[m]), sizeof(struct mode_s));
			if (minor == curminor)
				apply_gime(minor);
			return 0;
		}
	case GFXIOC_DRAW:
	case GFXIOC_WRITE:
	case GFXIOC_READ:
		{
			int err;
			err = gfx_draw_op(arg, ptr);
			if (err) {
				udata.u_error = err;
				err = -1;
			}
			return err;
		}
	default:
		break;
	}

	udata.u_error = ENOTTY;
	return -1;

      inval:udata.u_error = EINVAL;
	return -1;
}


/* Initial Setup stuff down here. */

uint8_t rgb_def_pal[16] = {
	0, 8, 32, 40, 16, 24, 48, 63,
	0, 8, 32, 40, 16, 24, 48, 63
};


static void apply_defmode(uint8_t defmode)
{
	int i;
	for (i = 0; i < 2; i++) {
		memcpy(&(ttytab[i].vmod), &(mode[defmode]), sizeof(struct mode_s));
	}
	apply_gime(1);
}

__attribute__((section(".discard")))
void set_defmode(char *s)
{
	int defmode = s[7] - 0x30;
	if (defmode > 4)
		defmode = 0;
	apply_defmode(defmode);
}

__attribute__((section(".discard")))
void devtty_init()
{
	/* set default keyboard delay/repeat rates */
	keyrepeat.first = REPEAT_FIRST * (TICKSPERSEC / 10);
	keyrepeat.continual = REPEAT_CONTINUAL * (TICKSPERSEC / 10);
	apply_defmode(0);
	/* make video palettes match vt.h's definitions. */
	memcpy((uint8_t *) 0xffb0, rgb_def_pal, 16);
	vt_load(&curtty->vt);
}
