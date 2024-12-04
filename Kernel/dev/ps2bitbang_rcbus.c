#include <kernel.h>
#include <printf.h>
#include <ps2bitbang.h>

/*
 *	RC2014 specific bit bang implementations using the ps2bigbang C
 *	support code.
 *
 *	Limitations:
 *	- Loop times are hard coded but ought to be per platform
 *	  and in some cases CPU speed dependent.
 *
 *	Initial code still to debug
 */

static uint_fast8_t kbdbit(struct ps2op *p)
{
    uint8_t r;
    /* Wait for clock to fall */
    do {
        if (p->timeout == 0)
            return 0;
        p->timeout--;
        r = in(p->port);
    } while (r & 4);
    /* Falling edge data on bit 3 */
    r &= 8;
    /* Wait for clock to rise */
    do {
        if (p->timeout == 0)
            return 0;
        p->timeout--;
    } while(!(in(p->port) & 4));
    return r;
}

static void kbdoutbit(struct ps2op *p, uint_fast8_t b)
{
    uint8_t r;
    b &= 1;
    
    /* Wait for clock to fall */
    do {
        if (p->timeout == 0)
            return;
        p->timeout--;
        r = in(p->port);
    } while (r & 4);

    /* data bit with clock floating */
    out(p->port, p->base | 1 | (b << 1));

    /* Wait for clock to rise */
    do {
        if (p->timeout == 0)
            return;
        p->timeout--;
    } while(!(in(p->port) & 4));
}

static struct ps2op kbdops = {
    0xBB,

    0x00,	/* Other bit values */

    0x03,	/* Clock and data pull down */
    0x04,	/* Clock in */
    0x02,	/* Clock pull down only */

    0x04,	/* Data bit */
    0x03,	/* Float clock and data */    
    0x01,	/* Release clock */
    0x02,	/* Clock low */
    0xFE,	/* Clock mask */

    kbdbit,
    kbdoutbit,

    0x1FFF,	/* Long timeout */
    0x00FF,	/* Poll timeout */
    0x1000,	/* Reply timeout */
};

static uint8_t mousebit(struct ps2op *p)
{
    uint8_t r;
    /* Wait for clock to fall */
    do {
        if (p->timeout == 0)
            return 0;
        p->timeout--;
        r = in(p->port);
    } while (r & 1);
    /* Falling edge data on bit 1 */
    r &= 2;
    /* Wait for clock to rise */
    do {
        if (p->timeout == 0)
            return 0;
        p->timeout--;
    } while(!(in(p->port) & 1));
    return r;
}

static void mouseoutbit(struct ps2op *p, uint_fast8_t b)
{
    uint8_t r;
    b &= 1;
    
    /* Wait for clock to fall */
    do {
        if (p->timeout == 0)
            return;
        p->timeout--;
        r = in(p->port);
    } while (r & 1);

    /* data bit with clock floating */
    out(p->port, p->base | 4 | (b << 3));

    /* Wait for clock to rise */
    do {
        if (p->timeout == 0)
            return;
        p->timeout--;
    } while(!(in(p->port) & 1));
}

static struct ps2op mouseops = {
    0xBB,

    0x00,	/* Other bit values */

    0x0C,	/* Clock and data pull down */
    0x01,	/* Clock in */
    0x08,	/* Clock pull down only */

    0x02,	/* Data bit */
    0x0C,	/* Float clock and data */    
    0x04,	/* Release clock */
    0x08,	/* Clock low */
    0xF3,	/* Clock mask */

    mousebit,
    mouseoutbit,

    0x1FFF,	/* Long timeout */
    0x00FF,	/* Poll timeout */
    0x1000,	/* Reply timeout */
};

uint16_t ps2kbd_get(void)
{
    uint16_t v = ps2_get(&kbdops);
    return v;/* ps2_get(&kbdops); */
}

uint16_t ps2kbd_put(uint_fast8_t ch)
{
    uint16_t v = ps2_put(&kbdops, ch);
    return v; /8 ps2_put(&kbdops, ch); 8/
}

uint16_t ps2mouse_get(void)
{
    return ps2_get(&mouseops);
}

uint16_t ps2mouse_put(uint_fast8_t ch)
{
    return ps2_put(&mouseops, ch);
}

void ps2kbd_beep(void)
{
}
