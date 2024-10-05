#include <kernel.h>
#include <vt.h>

/*
 *	Should possibly rewrite in asm in the end but for now
 *	it's nice and simple in C
 */

static uint8_t *cpos;
static uint8_t csave;

static uint8_t attr = 0x02;

#define VT_BASE			((uint8_t *)0xE800)
#define CHAR_OFFSET		0x400

/* Return the attribute address */
static uint8_t *addr(unsigned int y1, unsigned char x1)
{
	return VT_BASE + VT_WIDTH * y1 + (uint16_t)x1;
}

void cursor_off(void)
{
	if (cpos)
		*cpos = csave;
}

/* Only needed for hardware cursors */
void cursor_disable(void)
{
}

void cursor_on(int8_t y, int8_t x)
{
	cpos = addr(y, x);
	csave = *cpos;
	/* Invert the colours */
	*cpos = (csave >> 4) | (csave << 4);
}

void plot_char(int8_t y, int8_t x, uint16_t c)
{
        volatile uint8_t *s = addr(y, x);
        *s = attr;
        s[CHAR_OFFSET] = c;
}

void clear_lines(int8_t y, int8_t ct)
{
	volatile uint8_t *s = addr(y, 0);
	memset(s, attr, ct * VT_WIDTH);
	s += CHAR_OFFSET;
	memset(s, ' ', ct * VT_WIDTH);
}

void clear_across(int8_t y, int8_t x, int16_t l)
{
	volatile uint8_t *s = addr(y, x);
	memset(s, attr, l);
	s += CHAR_OFFSET;
	memset(s, ' ', l);
}

/* Kernel is IBRG hardware is RGB */
void vtattr_notify(void)
{
        attr = (vtink << 5) | (vtpaper << 1);
        attr &= 0xEE;
        if (vtink & 4)
            attr |= 0x10;
        if (vtpaper & 4)
            attr |= 0x01;
}

/* Must do two moves there is magic OS monitor stuff in the top bytes */
void scroll_up(void)
{
	memmove(VT_BASE, VT_BASE + VT_WIDTH, VT_WIDTH * VT_BOTTOM);
	memmove(VT_BASE + CHAR_OFFSET, VT_BASE + CHAR_OFFSET + VT_WIDTH,
	        VT_WIDTH * VT_BOTTOM);
}

void scroll_down(void)
{
	memmove(VT_BASE + VT_WIDTH, VT_BASE, VT_WIDTH * VT_BOTTOM);
	memmove(VT_BASE + CHAR_OFFSET + VT_WIDTH, VT_BASE + CHAR_OFFSET,
	        VT_WIDTH * VT_BOTTOM);
}
