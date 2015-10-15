#include <kernel.h>
#include <devtty.h>

extern video_cmd( char *rlt_data);

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

// Get copy user buffer to video mem
// 
static void video_get( char *usrptr ){
	map_for_video();
	uget( usrptr, (char *)0x2000, 512);
	map_for_kernel();
}


void video_init( )
{
	map_for_video();
	memset( (char *)0x2000, ' ', 0x4000 );
	map_for_kernel();
}

int gfx_draw_op(uarg_t arg, char *ptr)
{
	int err=0;
	int l;
	int c = 8;	/* 4 x uint16_t */
	uint16_t *p = (uint16_t *)(char *)0x5e00;

	map_for_video();
	l = ugetw(ptr);
	if (l < 6 || l > 512){
		err = EINVAL;
		goto ret;
	}
	if (arg != GFXIOC_READ)
		c = l;
	if (uget(ptr + 2, (char *)0x5e00, c)){
		err = EFAULT;
		goto ret;
	}
	switch(arg) {
	case GFXIOC_DRAW:
		/* TODO
		   if (draw_validate(ptr, l, 256, 192))  - or 128!
		   return EINVAL */
		video_cmd( ( char *)0x5e00 );
		break;
	case GFXIOC_WRITE:
	case GFXIOC_READ:
		err = EFAULT;
		goto ret;
		/* Not implemented yet....
		   if (l < 8)
		   return EINVAL;
		   l -= 8;
		   if (p[0] > 31 || p[1] > 191 || p[2] > 31 || p[3] > 191 ||
		   p[0] + p[2] > 32 || p[1] + p[3] > 192 ||
		   (p[2] * p[3]) > l)
		   return -EFAULT;
		   if (arg == GFXIOC_READ) {
		   video_read(buf);
		   if (uput(buf + 8, ptr, l))
		   return EFAULT;
		   return 0;
		   }
		   video_write(buf);
		*/
	}
 ret:
	map_for_kernel();
	return err;
}
