#include <kernel.h>
#include <tty.h>
#include <vt.h>

#include <devtty.h>

unsigned char vt_mangle_6847(unsigned char c);

/* Use macros so that functions are kept identical to Kernel/vt.c */
#undef VT_MAP_CHAR
#define VT_MAP_CHAR(x)	vt_mangle_6847(x)
#define VT_BASE		((uint8_t *) VC_BASE + 0x200 * (curtty - 2))
#define VT_WIDTH	32
#define VC		(curtty - 2)

static unsigned char *cpos[2];
static unsigned char csave[2];

static uint8_t *char_addr(uint8_t y1, uint8_t x1)
{
	return VT_BASE + VT_WIDTH * y1 + x1;
}

void vc_cursor_off(void)
{
	if (cpos[VC])
		*cpos[VC] = csave[VC];
}

void vc_cursor_on(int8_t y, int8_t x)
{
	cpos[VC] = char_addr(y, x);
	csave[VC] = *cpos[VC];
	*cpos[VC] = 0x80;		/* black square */
}

void vc_plot_char(int8_t y, int8_t x, uint16_t c)
{
	*char_addr(y, x) = VT_MAP_CHAR(c);
}

void vc_clear_lines(int8_t y, int8_t ct)
{
	unsigned char *s = char_addr(y, 0);
	memset(s, ' ', ct * VT_WIDTH);
}

void vc_clear_across(int8_t y, int8_t x, int16_t l)
{
	unsigned char *s = char_addr(y, x);
	memset(s, ' ', l);
}

void vc_vtattr_notify(void)
{
}

/* FIXME: these should use memmove */

void vc_scroll_up(void)
{
	memcpy(VT_BASE, VT_BASE + VT_WIDTH, VT_WIDTH * VT_BOTTOM);
}

void vc_scroll_down(void)
{
	memcpy(VT_BASE + VT_WIDTH, VT_BASE, VT_WIDTH * VT_BOTTOM);
}

unsigned char vt_mangle_6847(unsigned char c)
{
	if (c >= 96)
		c -= 32;
	c &= 0x3F;
	return c;
}

/* called without curtty and VC being set - using VC_BASE */
void vc_clear(int8_t vc_num)
{
	memset((uint8_t*)VC_BASE + 0x200 * vc_num, ' ', 0x200);
}
