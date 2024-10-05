/*
 *	Starting to flesh out the NEC 7220 support
 *
 *	The actual vt layer needs
 *	plot character
 *	scroll up/down (in chars)
 *	clear lines
 *	clear to end of line
 *	init
 *
 *	bonus points for colour, italic, bold etc
 */

#include <kernel.h>

__sfr __at 0x90 gdc_p;
__sfr __at 0x90	gdc_s;
__sfr __at 0x91 gdc_c;
__sfr __at 0x91 gdc_r;

static uint16_t ybuf;

/* Keep it all in little tables. It's more verbose source, but it makes the binary
   tiny */
static uint8_t sync640_op[] = {
    0x0F,
    8,		/* 8 bytes of parameter data */
    0x16,	/* Graphics */
    0x26,	/* 0x28 x 0x10 bits -> 640 pixels */
    0x44,	/* HS 5, VSL 2 */
    0x04,	/* HFP 2 VShigh 0 */
    0x02,	/* HBP 3 */
    0x0A,	/* 10 lines */
    0xE0,	/* low bits of 480 */
    0x85	/* high bits of 480 + VBP of 33 */
};
 
static uint8_t no_chr_op[] = {
    0x4B,
    3,
    0, 0, 0
};

static uint8_t pitch640_op[] = {
    0x47,
    0x28
};

static uint8_t pattern_op[] = {
    0x4A,
    2,
    0xFF,
    0xFF
};

static uint8_t nozoom_op[] = {
    0x46,
    1,
    0
};

static uint8_t cursor_op[] = {
    0x49,
    3,
    0,
    0
};

static void n7220_waitfifo(void)
{
    /* Needs a timeout ? */
    while(gdc_s & 0x02);
}

/*
 *	Issue a command via PIO
 */
static void n7220_op(uint8_t *op)
{
    uint_fast8_t n;
    n7220_waitfifo();
    gdc_c = *op++;
    n = *op++;
    while(n--) {
        n7220_waitfifo();
        gdc_p = *op++;
    }
}

/* Issue a simple command */
static void n7220_cmd(uint8_t cmd)
{
    n7220_waitfifo();
    gdc_c = cmd;
}

static uint8_t abuf[] = {
    0x70,
    4,
    0, 0, 0, 0
};

static void n7220_set_area(uint_fast8_t area, uint32_t addr, uint16_t len)
{
    abuf[2] = addr;
    abuf[3] = addr >> 8;
    abuf[4] = ((addr >> 16) & 0x03) | ((len & 0x0F) << 4);
    abuf[5] = len >> 4;
    abuf[0] = area;
    n7220_op(abuf);
}

static void n7220_set_split(uint16_t split)
{
    if (split == 0)
        n7220_set_area(0x70, 0, ybuf);
    else
        n7220_set_area(0x70, (ybuf - split) * 0x28, split);
    n7220_set_area(0x74, 0, ybuf - split);
}

void n7220_init(void)
{
    n7220_cmd(3);	/* Reset */
    n7220_op(sync640_op);
    n7220_cmd(0x6F);
    n7220_op(no_chr_op);	/* We have no character mode */
    n7220_op(pitch640_op);
    n7220_set_split(0);		/* Set up the display windows */
    n7220_op(pattern_op);	/* Solid */
    n7220_op(nozoom_op);	/* No zoom on draw or display */
    n7220_op(cursor_op);	/* Cursor to 0,0 */
    n7220_cmd(0x20);		/* Plot mode */
    n7220_cmd(0x6B);		/* Go */
    /* Should wipe display here */
}

/*
 *	The kernel VT interface. This operates entirely in characters and has
 *	no internal idea about pixels and pixel mapping
 */

uint8_t vtattr_cap = 0;		/* for now no attribute features */

/* Scroll the display. Does not need to clear the new text line */
void scroll_down(void)
{
}

void scroll_up(void)
{
}

/* Clear n characters across from y,x - can be 0 */
void clear_across(int8_t y, int8_t x, int16_t n)
{
}

/* Clear ct lines from y (can be 0) */
void clear_lines(int8_t y, int8_t ct)
{
}

/* Beep if supported */
void do_beep(void)
{
}

/* Disable hardware cursor if present */
void cursor_disable(void)
{
}

/* Undraw software cursor */
void cursor_off(void)
{
}

/* Position (and for software also draw) the cursor. If needed also save what
   is under the cursor */
void cursor_on(int8_t y, int8_t x)
{
}

/* The attribute settings changed - mostly useful for runtime generated renderers */
void vtattr_notify(void)
{
}

/* Plot the character c at y,x */
void plot_char(int8_t y, int8_t x, uint16_t c)
{
}
