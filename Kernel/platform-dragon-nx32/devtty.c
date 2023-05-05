#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <device.h>
#include "devfd.h"
#include <vt.h>
#include <tty.h>
#include <devdw.h>
#include <ttydw.h>
#include <graphics.h>
#include <crt9128.h>
#include <input.h>

void set_vc_mode(uint8_t);
void set_vid_mode(void);

#undef  DEBUG			/* UNdefine to delete debug code sequences */

uint8_t *uart_data = (uint8_t *)0xFF04;		/* ACIA data */
uint8_t *uart_status = (uint8_t *)0xFF05;	/* ACIA status */
uint8_t *uart_command = (uint8_t *)0xFF06;	/* ACIA command */
uint8_t *uart_control = (uint8_t *)0xFF07;	/* ACIA control */

#define ACIA_TTY 5
#define is_dw(minor) (minor >= DW_MIN_OFF)

static unsigned char tbuf1[TTYSIZ];
static unsigned char tbuf2[TTYSIZ];
static unsigned char tbuf3[TTYSIZ];
static unsigned char tbuf4[TTYSIZ];
static unsigned char tbuf5[TTYSIZ];
static unsigned char tbuf6[TTYSIZ];   /* drivewire VSER 0 */
static unsigned char tbuf7[TTYSIZ];   /* drivewire VWIN 0 */

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf3, tbuf3, tbuf3, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf4, tbuf4, tbuf4, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf5, tbuf5, tbuf5, TTYSIZ, 0, TTYSIZ / 2},
	/* Drivewire Virtual Serial Ports */
	{tbuf6, tbuf6, tbuf6, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf7, tbuf7, tbuf7, TTYSIZ, 0, TTYSIZ / 2},
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	/* Console */
	_CSYS,
	/* 9128 */
	_CSYS,
	/* VC */
	_CSYS,
	_CSYS,
	/* ACIA */
	/* Review flow control and CSTOPB TODO */
	_CSYS|CBAUD|CSIZE|PARENB|PARODD|PARMRK,
	/* Drivewire */
	_CSYS,
	_CSYS
};

uint8_t vtattr_cap = VTA_INVERSE|VTA_UNDERLINE|VTA_ITALIC|VTA_BOLD|
		     VTA_OVERSTRIKE|VTA_NOCURSOR;

const signed char vt_tright[4]  = { 31, 77, 31, 31 };
const signed char vt_tbottom[4] = { 23, 22, 15, 15 };
extern uint8_t curtty;
static uint8_t inputtty;
static struct vt_switch ttysave[4];
static uint8_t vmode;
static uint8_t kbd_timer;
struct vt_repeat keyrepeat = { 40, 4 };

/* tty1 is the screen tty2 is the serial port */

/* Output for the system console (kprintf etc) */
void kputchar(uint_fast8_t c)
{
	if (c == '\n')
		tty_putc(TTYDEV & 0xff, '\r');
	tty_putc(TTYDEV & 0xff, c);
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
	uint8_t c = 0xff;
	if (minor == 1)
		return TTY_READY_NOW;
	else if (minor == 2)
		c = crt9128_done();
	else if (minor == ACIA_TTY)
		c = *uart_status & 16; /* TX DATA empty */
	return c ? TTY_READY_NOW : TTY_READY_SOON;
}

/* For DragonPlus we should perhaps support both monitors 8) */

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	irqflags_t irq;

	if (is_dw(minor)) {
		dw_putc(minor, c);
		return;
	} else if (minor == ACIA_TTY) {
		*uart_data = c;	/* Data */
		return;
	}
	irq = di();
	if (curtty != minor - 1) {
		vt_save(&ttysave[curtty]);
		curtty = minor - 1;
		vt_load(&ttysave[curtty]);
	}
	if (minor == 1) {
		/* We don't do text except in 256x192 resolution modes */
		if (vmode < 2)
			vtoutput(&c, 1);
	} else if (minor >= 2 && minor <= 4) {
		vtoutput(&c, 1);
	}
	irqrestore(irq);
}

void tty_sleeping(uint_fast8_t minor)
{
    used(minor);
}

/* 6551 mode handling */

static uint8_t baudbits[] = {
	0x11,		/* 50 */
	0x12,		/* 75 */
	0x13,		/* 110 */
	0x14,		/* 134 */
	0x15,		/* 150 */
	0x16,		/* 300 */
	0x17,		/* 600 */
	0x18,		/* 1200 */
	0x1A,		/* 2400 */
	0x1C,		/* 4800 */
	0x1E,		/* 9600 */
	0x1F,		/* 19200 */
};

static uint8_t bitbits[] = {
	0x30,
	0x20,
	0x10,
	0x00
};

void tty_setup(uint_fast8_t minor, uint_fast8_t flag)
{
	uint8_t r;
	if (is_dw(minor)) {
		dw_vopen(minor);
		return;
	}
	if (minor == 2) {
		crt9128_init();
		return;
	}
	if (minor == 3 || minor == 4) {
		vc_clear(minor - 3);
	}
	if (minor != ACIA_TTY)
		return;
	r = ttydata[ACIA_TTY].termios.c_cflag & CBAUD;
	if (r > B19200) {
		r = 0x1F;	/* 19.2 */
		ttydata[ACIA_TTY].termios.c_cflag &=~CBAUD;
		ttydata[ACIA_TTY].termios.c_cflag |= B19200;
	} else
		r = baudbits[r];
	r |= bitbits[(ttydata[ACIA_TTY].termios.c_cflag & CSIZE) >> 4];
	*uart_control = r;
	r = 0x0A;	/* rx and tx on, !rts low, dtr int off, no echo */
	if (ttydata[ACIA_TTY].termios.c_cflag & PARENB) {
		if (ttydata[ACIA_TTY].termios.c_cflag & PARODD)
			r |= 0x20;	/* Odd parity */
		else
			r |= 0x60;	/* Even parity */
		if (ttydata[ACIA_TTY].termios.c_cflag & PARMRK)
			r |= 0x80;	/* Mark/space */
	}
}

int tty_carrier(uint_fast8_t minor)
{
	if (is_dw(minor)) return dw_carrier( minor );
	/* The serial DCD is status bit 5 but not wired */
	return 1;
}

void tty_data_consumed(uint_fast8_t minor)
{
}

void tty_interrupt(void)
{
	uint8_t r = *uart_status;
	if (r & 0x8) {
		r = *uart_data;
		tty_inproc(ACIA_TTY,r);
	}	
}

uint8_t keymap[8];
static uint8_t keyin[8];
static uint8_t keybyte, keybit;
static uint8_t newkey;
static int keysdown = 0;
/* FIXME: shouldn't COCO shiftmask also differ ??? 0x02 not 0x40 ?? */
static uint8_t shiftmask[8] = {
	0, 0x40, 0, 0, 0, 0, 0, 0x40
};

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
static volatile uint8_t *pia_row = (uint8_t *)0xFF00;
/* Columns for scanning: multiplexed with the printer port */
static volatile uint8_t *pia_col = (uint8_t *)0xFF02;
/* Control */
static volatile uint8_t *pia_ctrl = (uint8_t *)0xFF03;

static uint8_t keyany(void)
{
	uint8_t x;
	*pia_col = 0x00;
	x = ~*pia_row;
	*pia_col = 0xFF;
	return x;
}

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
					if (!(shiftmask[i] & m)) {
						if (keyboard_grab == 3) {
							queue_input(KEYPRESS_UP);
							queue_input(keyboard[i][n]);
						}
						keysdown--;
					}
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
	if (newkey == 1 && system_id && keybit != 6) { 	/* COCO series */
	  keybit += 2;
	  if (keybit > 5)
	    keybit -= 6;
        }
}

uint8_t keyboard[8][7] = {
	{ '0', '8', '@', 'h', 'p', 'x', KEY_ENTER },
	{ '1', '9', 'a', 'i', 'q', 'y', 0 /* clear - used as ctrl*/ },
	{ '2', ':', 'b', 'j', 'r', 'z', KEY_ESC /* break (used for esc) */ },
	{ '3', ';', 'c', 'k', 's', '^' /* up */, 0 /* NC */ },
	{ '4', ',', 'd', 'l', 't', '|' /* down */, 0 /* NC */ },
	{ '5', '-', 'e', 'm', 'u', KEY_BS /* left */, 0 /* NC */ },
	{ '6', '.', 'f', 'n', 'v', KEY_TAB /* right */, 0 /* NC */ },
	{ '7', '/', 'g', 'o', 'w', ' ', 0 /* shift */ },
};

uint8_t shiftkeyboard[8][7] = {
	{ '_', '(', '\\', 'H', 'P', 'X', KEY_ENTER },
	{ '!', ')', 'A', 'I', 'Q', 'Y', 0 /* clear - used as ctrl*/ },
	{ '"', '*', 'B', 'J', 'R', 'Z', CTRL('C') /* break */ },
	{ '#', '+', 'C', 'K', 'S', '[' /* up */, 0 /* NC */ },
	{ '$', '<', 'D', 'L', 'T', ']' /* down */, 0 /* NC */ },
	{ '%', '=', 'E', 'M', 'U', '{' /* left */, 0 /* NC */ },
	{ '&', '>', 'F', 'N', 'V', '}' /* right */, 0 /* NC */ },
	{ '\'', '?', 'G', 'O', 'W', ' ', 0 /* shift */ },
};

static void keydecode(void)
{
	uint8_t m = 0;
	uint8_t c;

	if (keymap[7] & 64) { /* shift */
		c = shiftkeyboard[keybyte][keybit];
		m = KEYPRESS_SHIFT;
	} else
		c = keyboard[keybyte][keybit];
	if (keymap[1] & 64) {	/* control */
		m |= KEYPRESS_CTRL;
		if (c >= '1' && c <= '4') {
			inputtty = c - '1';
			/* switch VDU base and VDG/SAM video mode */
			if (inputtty == 0)
				set_vid_mode();
			else if (inputtty == 2 || inputtty == 3)
				set_vc_mode(inputtty - 2);
			return;
		}
		if (c > 31 && c < 127)
			c &= 31;
	}
	if (c) {
		switch(keyboard_grab) {
		case 0:
			tty_inproc(inputtty + 1, c);
			break;
		case 1:
			if (!input_match_meta(c)) {
				vt_inproc(inputtty + 1, c);
				break;
			}
			/* Fall through */
		case 2:
			queue_input(KEYPRESS_DOWN);
			queue_input(c);
			break;
		case 3:
			/* Queue an event giving the base key (unshifted)
			   and the state of shift/ctrl/alt */
			queue_input(KEYPRESS_DOWN | m);
			queue_input(keyboard[keybyte][keybit]);
			break;
		}
	}
}

uint8_t sys_hz;
static uint8_t vblank_wait;

void do_plt_interrupt(uint_fast8_t re)
{
	static uint8_t tick;
	uint8_t i = *pia_ctrl;
	if (i & 0x80) {
		if (vblank_wait)
			wakeup(&vblank_wait);
		/* Do a single keyboard scan of all lines first if we have no key down> This makes
		   the usual case much faster and saves precious interrupt time clocks */
		if (keysdown || keyany()) {
			*pia_col;
			newkey = 0;
			keyproc();
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
                fd_timer_tick();
		tick++;
		if (re == 0) {
			while (tick >= sys_hz) {
				tick -= sys_hz;
				timer_interrupt();
			}
		}
		wakeup(&plt_interrupt);
		dw_vpoll();
	}
}

void plt_interrupt(void)
{
	do_plt_interrupt(0);
}

/* We don't currently make use of this but we could improve fork and swap this way */
void plt_reinterrupt(void)
{
	do_plt_interrupt(1);
}

/* This is used by the vt asm code, but needs to live at the top of the kernel */
uint16_t cursorpos;

static struct display display[4] = {
	/* Two variants of 256x192 with different palette */
	/* 256 x 192 */
	{
		0,
		256, 192,
		256, 192,
		0xFF, 0xFF,		/* For now */
		FMT_MONO_WB,
		HW_UNACCEL,
		GFX_TEXT|GFX_MAPPABLE|GFX_VBLANK|GFX_MULTIMODE,
		0,
		GFX_DRAW|GFX_READ|GFX_WRITE,
	},
	{
		1,
		256, 192,
		256, 192,
		0xFF, 0xFF,		/* For now */
		FMT_MONO_WB,
		HW_UNACCEL,
		GFX_TEXT|GFX_MAPPABLE|GFX_VBLANK|GFX_MULTIMODE,
		0,
		GFX_DRAW|GFX_READ|GFX_WRITE,
	},
	/* 128 x 192 four colour modes */
	{
		2,
		128, 192,
		128, 192,
		0xFF, 0xFF,		/* For now */
		FMT_COLOUR4,
		HW_UNACCEL,
		GFX_MAPPABLE|GFX_VBLANK,
		0,
		GFX_DRAW|GFX_READ|GFX_WRITE|GFX_MULTIMODE,
	},
	{
		3,
		128, 192,
		128, 192,
		0xFF, 0xFF,		/* For now */
		FMT_COLOUR4,
		HW_UNACCEL,
		GFX_MAPPABLE|GFX_VBLANK,
		0,
		GFX_DRAW|GFX_READ|GFX_WRITE|GFX_MULTIMODE,
	},
	/* Possibly we should also allow for SG6 and SG4 ?? */
};

static struct videomap displaymap = {
	0,
	0,
	VIDEO_BASE,
	6 * 1024,
	0,
	0,
	0,
	MAP_FBMEM|MAP_FBMEM_SIMPLE
};

/* bit 7 - A/G 6 GM2 5 GM1 4 GM0 & INT/EXT 3 CSS */
static uint8_t piabits[] = { 0xF0, 0xF8, 0xE0, 0xE8};
///* V0 V1 V2 */
//static uint8_t sambits[] = { 0x6, 0x6, 0x6, 0x6 };

static struct fontinfo fontinfo = {
	0, 255, 128, 255, FONT_INFO_8X8
};

#define pia1b	((volatile uint8_t *)0xFF22)
#define sam_v	((volatile uint8_t *)0xFFC0)

static int gfx_draw_op(uarg_t arg, char *ptr, uint8_t *buf)
{
  int l;
  int c = 8;	/* 4 x uint16_t */
  uint16_t *p = (uint16_t *)buf;
  l = ugetw(ptr);
  if (l < 6 || l > 512)
    return EINVAL;
  if (arg != GFXIOC_READ)
    c = l;
  if (uget(buf, ptr + 2, c))
    return EFAULT;
  switch(arg) {
  case GFXIOC_DRAW:
    /* TODO
    if (draw_validate(ptr, l, 256, 192))  - or 128!
      return EINVAL */
    video_cmd(buf);
    break;
  case GFXIOC_WRITE:
  case GFXIOC_READ:
    if (l < 8)
      return EINVAL;
    l -= 8;
    if (p[0] > 191 || p[1] > 31 || p[2] > 192 || p[3] > 32 ||
      p[0] + p[2] > 192 || p[1] + p[3] > 32 ||
      (p[2] * p[3]) > l)
      return -EFAULT;
    if (arg == GFXIOC_READ) {
      video_read(buf);
      if (uput(buf + 8, ptr + 10, l - 2))
        return EFAULT;
      return 0;
    }
    video_write(buf);
  }
  return 0;
}

int gfx_ioctl(uint_fast8_t minor, uarg_t arg, char *ptr)
{
	extern unsigned char fontdata_8x8[];

	if (is_dw(minor))	/* remove once DW get its own ioctl() */
		return tty_ioctl(minor, arg, ptr);
	if (minor == 1) {
		uint16_t size = 128 * 8;
		uint16_t base = 128 * 8;
		switch (arg) {
		case VTFONTINFO:
			return uput(&fontinfo, ptr, sizeof(fontinfo));
		case VTSETFONT:
			size = base = 0;
		case VTSETUDG:
			return uget(fontdata_8x8 + base, ptr, size);
		case VTGETFONT:
			size = base = 0;
		case VTGETUDG:
			return uput(fontdata_8x8 + base, ptr, size);
		}
	}
	if (arg == VTSIZE)
		return (vt_tbottom[minor - 1] + 1) << 8 | (vt_tright[minor - 1] + 1);
	if (arg >> 8 != 0x03)
		return vt_ioctl(minor, arg, ptr);

	/* FIXME: limit to console ? */

	switch(arg) {
	case GFXIOC_GETINFO:
		return uput(&display[vmode], ptr, sizeof(struct display));
	case GFXIOC_MAP:
		return uput(&displaymap, ptr, sizeof(displaymap));
	case GFXIOC_UNMAP:
		return 0;
	case GFXIOC_GETMODE:
	case GFXIOC_SETMODE:
	{
		uint8_t m = ugetc(ptr);
//		uint8_t b;
		if (m > 3) {
			udata.u_error = EINVAL;
			return -1;
		}
		if (arg == GFXIOC_GETMODE)
			return uput(&display[m], ptr, sizeof(struct display));
		vmode = m;
		*pia1b = (*pia1b & 0x07) | piabits[m];
//		b = sambits[m];
//		sam_v[b & 1] = 0;
//		sam_v[(b & 2)?3:2] = 0;
//		sam_v[(b & 4)?5:4] = 0;
		return 0;
	}
	case GFXIOC_WAITVB:
		vblank_wait++;
		psleep(&vblank_wait);
		vblank_wait--;
		chksigs();
		if (udata.u_cursig) {
			udata.u_error = EINTR;
				return -1;
		}
		return 0;
	case GFXIOC_DRAW:
	case GFXIOC_READ:
	case GFXIOC_WRITE:
	{
		uint8_t *tmp;
		int err;

		tmp = (uint8_t *)tmpbuf();
		err = gfx_draw_op(arg, ptr, tmp);
		tmpfree(tmp);
		if (err) {
			udata.u_error = err;
			err = -1;
		}
		return err;
	}
	}
	return -1;
}

/* A wrapper for tty_close that closes the DW port properly */
int my_tty_close(uint_fast8_t minor)
{
	if (is_dw(minor) && ttydata[minor].users == 1)
		dw_vclose(minor);
	return (tty_close(minor));
}
