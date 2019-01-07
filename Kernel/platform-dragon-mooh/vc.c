#include <kernel.h>
#include <tty.h>
#include <vt.h>

#include <devtty.h>
#include "vc_asm.h"

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
		vc_write_char(cpos[VC], csave[VC]);
}

void vc_cursor_on(int8_t y, int8_t x)
{
	cpos[VC] = char_addr(y, x);
	csave[VC] = vc_read_char(cpos[VC]);
	vc_write_char(cpos[VC], 0x80);	/* black square */
}

void vc_plot_char(int8_t y, int8_t x, uint16_t c)
{
	vc_write_char(char_addr(y, x), VT_MAP_CHAR(c));
}

void vc_clear_lines(int8_t y, int8_t ct)
{
	unsigned char *s = char_addr(y, 0);
	vc_memset(s, ' ', ct * VT_WIDTH);
}

void vc_clear_across(int8_t y, int8_t x, int16_t l)
{
	unsigned char *s = char_addr(y, x);
	vc_memset(s, ' ', l);
}

void vc_vtattr_notify(void)
{
}

unsigned char vt_mangle_6847(unsigned char c)
{
	if (c >= 96)
		c -= 32;
	c &= 0x3F;
	return c;
}
