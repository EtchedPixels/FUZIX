#include <kernel.h>
#include <vt.h>
#include <devtty.h>

/*
 *	VDP driver. For the 68000 we can do this sensibly in C
 *
 *	FIXME: need to look at IRQ locking if this is console or timer. Should
 *	be mostly ok with thought.
 */

static uint16_t cursorpos;
static uint8_t cursorpeek;

static void vdpout(uint16_t v)
{
    out(0x99, v);
    v >>= 8; 
    out(0x99, v);
}

#define MODE_READ	0x0000
#define MODE_WRITE	0x4000

static uint16_t videopos(uint8_t y, uint8_t x, uint16_t m)
{
    return (y * 40 + x) | m;
}

void plot_char(int8_t y, int8_t x, uint16_t c)
{
    vdpout(videopos(y,x, MODE_WRITE));
    out(0x98, c);
}

static uint8_t scrollbuf[40];

void scroll_down(void)
{
    uint8_t ct = 23;
    uint8_t x;
    uint16_t offset = 0x3C0;	/* Start of bottom line */
    uint8_t *sp;

    do {
        vdpout(offset);
        sp = scrollbuf;
        for (x = 0; x < 40; x++)
            *sp++ = in(0x98);
        vdpout((offset + 40) | MODE_READ);
        sp = scrollbuf;
        for (x = 0; x< 40; x++)
            out(0x98, *sp++);
        offset -= 40;
    } while(ct--);
}

void scroll_up(void)
{
    uint8_t ct = 23;
    uint8_t x;
    uint16_t offset = 0x0040;	/* Start of second line */
    uint8_t *sp;

    do {
        vdpout(offset);
        sp = scrollbuf;
        for (x = 0; x < 40; x++)
            *sp++ = in(0x98);
        vdpout((offset - 40) | MODE_READ);
        sp = scrollbuf;
        for (x = 0; x< 40; x++)
            out(0x98, *sp++);
        offset += 40;
    } while(ct--);
}

void clear_lines(int8_t y, int8_t ct)
{
    uint16_t offset = videopos(y, 0, MODE_WRITE);
    uint8_t x;
    vdpout(offset);
    while (ct--) {
        /* Will wrap nicely down lines */
        for (x = 0; x < 40; x++)
            out(0x98, ' ');
    }
            
}

void clear_across(int8_t y, int8_t x, int16_t l)
{
    uint16_t offset = videopos(y, x, MODE_WRITE);
    vdpout(offset);
    while(l--)
        out(0x98, ' ');
}

void cursor_off(void)
{
    vdpout(cursorpos);
    out(0x98, cursorpeek);
}

void cursor_on(int8_t y, int8_t x)
{
    cursorpos = videopos(y, x, MODE_READ);
    vdpout(cursorpos);
    cursorpeek = in(0x98);
    cursorpos |= MODE_WRITE;
    vdpout(cursorpos);
    out(0x98, cursorpeek ^ 0x80);	/* Invert */
}

void cursor_disable(void)
{
    /* Called within a cursor_off() state */
}

/* Multi-console support (will need keyboard drivers to use */

void set_console(void)
{
    vdpout(0x8200|inputtty);
}

/* No attribute support in text mode */
uint8_t vtattr_cap;

void vtattr_notify(void)
{
}

