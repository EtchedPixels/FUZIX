#include <kernel.h>
#include <vt.h>
#include <devtty.h>
#include <video.h>

extern void video_cmd(char *rlt_data);

/* These are routines stolen from the stock vt.c's VT_SIMPLE code, and modified
   to suite multiple vts

   These routines are called by vt.c.   They play with Kernel banking, so this
   module should be compiled into the .video section.  These routines should 
   only reference/call things that are in .video or .videodata

*/

static int irq;

static void map_for_video()
{
	irq = di();
	*(uint8_t *) 0xffa9 = 8;
	*(uint8_t *) 0xffaa = 9;
}

static void map_for_kernel()
{
	*(uint8_t *) 0xffa9 = 1;
	*(uint8_t *) 0xffaa = 2;
	irqrestore(irq);
}

static uint8_t *char_addr(unsigned int y1, unsigned char x1)
{
	return curtty->base + VT_WIDTH * y1 * 2 + (uint16_t) (x1 * 2);
}

void cursor_off(void)
{
	map_for_video();
	if (curtty->cpos)
		*curtty->cpos = curtty->csave;
	map_for_kernel();
}

void cursor_on(int8_t y, int8_t x)
{
	map_for_video();
	curtty->csave = *(char_addr(y, x) + 1);
	curtty->cpos = char_addr(y, x) + 1;
	*curtty->cpos = *curtty->cpos ^ 0x3f;
	map_for_kernel();
}

void cursor_disable(void)
{
}

void plot_char(int8_t y, int8_t x, uint16_t c)
{
	unsigned char *p = char_addr(y, x);
	map_for_video();
	*p++ = VT_MAP_CHAR(c);
	*p = curattr;
	map_for_kernel();
}

void clear_lines(int8_t y, int8_t ct)
{
	uint16_t wc = ct * VT_WIDTH;
	map_for_video();
	uint16_t *s = (uint16_t *) char_addr(y, 0);
	uint16_t w = ' ' * 0x100 + curattr;
	while (wc--)
		*s++ = w;
	map_for_kernel();
}

void clear_across(int8_t y, int8_t x, int16_t l)
{
	map_for_video();
	uint16_t *s = (uint16_t *) char_addr(y, x);
	uint16_t w = ' ' * 0x100 + curattr;
	while (l--)
		*s++ = w;
	map_for_kernel();
}

static void rmemcpy(unsigned char *dest, unsigned char *src, size_t n)
{
	unsigned char *d = dest + n;
	unsigned char *s = src + n;
	while (s != src)
		*--d = *--s;
}

void scroll_up(void)
{
	map_for_video();
	memcpy(curtty->base, curtty->base + VT_WIDTH * 2, VT_WIDTH * 2 * VT_BOTTOM);
	map_for_kernel();
}

void scroll_down(void)
{
	map_for_video();
	rmemcpy(curtty->base + VT_WIDTH * 2, curtty->base, VT_WIDTH * 2 * VT_BOTTOM);
	map_for_kernel();
}


unsigned char vt_map(unsigned char c)
{
	/* The CoCo3's gime has a strange code for underscore */
	if (c == '_')
		return 0x7F;
	if (c == '`')
		return 0x5E;	/* up arrow */
	return c;
}

// Get copy user buffer to video mem
// 
static void video_get(char *usrptr)
{
	map_for_video();
	uget(usrptr, (char *) 0x2000, 512);
	map_for_kernel();
}

__attribute__((section(".discard")))
void video_init()
{
	map_for_video();
	memset((char *) 0x2000, ' ', 0x4000);
	map_for_kernel();
}

int gfx_draw_op(uarg_t arg, char *ptr)
{
	int err = 0;
	int l;
	int c = 8;		/* 4 x uint16_t */
	uint16_t *p = (uint16_t *) (char *) 0x5e00;

	map_for_video();
	l = ugetw(ptr);
	if (l < 6 || l > 512) {
		err = EINVAL;
		goto ret;
	}
	if (arg != GFXIOC_READ)
		c = l;
	if (uget(ptr + 2, (char *) 0x5e00, c)) {
		err = EFAULT;
		goto ret;
	}
	switch (arg) {
	case GFXIOC_DRAW:
		/* TODO
		   if (draw_validate(ptr, l, 256, 192))  - or 128!
		   return EINVAL */
		video_cmd((char *) 0x5e00);
		break;
	case GFXIOC_WRITE:
	case GFXIOC_READ:
		if (l < 8) {
			err = EINVAL;
			break;
		}
		l -= 8;
		if (p[0] > 191 || p[1] > 31 || p[2] > 192 || p[3] > 32 || p[0] + p[2] > 192 || p[1] + p[3] > 32 || (p[2] * p[3]) > l) {
			err = -EFAULT;
			break;
		}
		if (arg == GFXIOC_READ) {
			video_read((unsigned char *) 0x5e00);
			if (uput((char *) 0x5e00 + 8, ptr + 10, l - 2))
				err = EFAULT;
			break;
		}
		video_write((unsigned char *) 0x5e00);
	}
      ret:
	map_for_kernel();
	return err;
}
