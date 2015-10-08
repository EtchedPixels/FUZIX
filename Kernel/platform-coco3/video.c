#include <kernel.h>
#include <devtty.h>



/* These are routines stolen from the stock vt.c's VT_SIMPLE code, and modified
   to suite multiple vts

   These routines are called by vt.c.   They play with Kernel banking, so this
   module should be compiled into the .video section.  These routines should 
   only reference/call things that are in .video or .videodata

*/

static void map_for_video()
{
	*( uint8_t *)0xffa9 = 8;
	*( uint8_t *)0xffaa = 9;
}

static void map_for_kernel()
{
	*( uint8_t *)0xffa9 = 1;
	*( uint8_t *)0xffaa = 2;
}

static uint8_t *char_addr(unsigned int y1, unsigned char x1)
{
	return curpty->base + VT_WIDTH * y1 + (uint16_t) x1;
}

void cursor_off(void)
{
	map_for_video();
	if (curpty->cpos)
		*curpty->cpos = curpty->csave;
	map_for_kernel();
}

void cursor_on(int8_t y, int8_t x)
{
	map_for_video();
	curpty->csave = *char_addr(y, x);
	curpty->cpos = char_addr(y, x);
	*curpty->cpos = VT_MAP_CHAR('_');
	map_for_kernel();
}

void plot_char(int8_t y, int8_t x, uint16_t c)
{
	map_for_video();
	*char_addr(y, x) = VT_MAP_CHAR(c);
	map_for_kernel();
}

void clear_lines(int8_t y, int8_t ct)
{
	map_for_video();
	unsigned char *s = char_addr(y, 0);
	memset(s, ' ', ct * VT_WIDTH);
	map_for_kernel();
}

void clear_across(int8_t y, int8_t x, int16_t l)
{
	map_for_video();
	unsigned char *s = char_addr(y, x);
	memset(s, ' ', l);
	map_for_kernel();
}

/* FIXME: these should use memmove */

void scroll_up(void)
{
	map_for_video();
	memcpy(curpty->base, curpty->base + VT_WIDTH,
	       VT_WIDTH * VT_BOTTOM);
	map_for_kernel();
}

void scroll_down(void)
{
	map_for_video();
	memcpy(curpty->base + VT_WIDTH, curpty->base,
	       VT_WIDTH * VT_BOTTOM);
	map_for_kernel();
}


unsigned char vt_map(unsigned char c)
{
	/* The CoCo3's gime has a strange code for underscore */
	if (c == '_')
		return 0x7F;
	if (c == '`')
		return 0x5E; /* up arrow */
	return c;
}
