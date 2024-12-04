#include <kernel.h>
#include <ps2bitbang.h>

/*
 *	Core code for the low level PS/2 bit banging. The actual
 *	bit bang methods live in per platform files
 *
 *	Initial code still to debug
 */

static uint16_t ps2_getbits(struct ps2op *p)
{
    uint8_t parity = 0;
    uint8_t r = 0;
    uint8_t i;

    p->timeout = p->long_timeout;

    if (p->bit(p) == 0)
        return 0xFFFC;		/* No start bit ? */

    for (i = 0; i < 8; i++) {
        r <<= 1;
        if (p->bit(p)) {
            r |= 1;
            parity++;
        }
    }
    /* Parity bit */
    if (p->bit(p))
        parity++;
    if (parity & 1) {
        /* Parity error */
        return 0xFFFE;
    }
    if (p->timeout == 0)
        return 0xFFFF;

    /* Must read the stop bit before returning */
    p->bit(p);		/* Stop bit */
    return r;
}
    
uint16_t ps2_get(struct ps2op *p)
{
    uint8_t r;

    p->timeout = p->poll_timeout;
    out(p->port, p->base|p->clockdata);
    do {
        r = in(p->port);
        if (!(r & p->clockin)) {
            return ps2_getbits(p);
        }
    } while(p->timeout--);
    out(p->port, p->base | p->clockonly);
    /* Timeout */
    return 0xFFFF;
}

/*
 *	Send a byte to the device
 */
uint16_t ps2_put(struct ps2op *p, uint8_t c)
{
    uint8_t parity = 0;
    uint8_t i;

    p->timeout = p->long_timeout;
    /* Pull clock low */
    out(p->port, (p->base & p->clockmask)|p->clocklow);
    napus(125);
    /* Release clock */
    out(p->port, (p->base & p->clockmask)|p->clockrel);
    /* Send data */
    for (i = 0; i < 8; i++) {
        p->sendbit(p, c);
        if (c & 1)
            parity++;
        c >>= 1;		/* FIXME check bit order */
    }
    /* Parity bit */
    p->sendbit(p, parity);
    /* Stop bits */
    p->sendbit(p, 1);
    p->sendbit(p, 1);
    
    napus(20);
    
    /* Clock low, data floating */
    out(p->port, p->base);    

    napus(44);

    out(p->port, (p->base & ~p->clockmask) | p->floatboth);

    p->timeout = p->reply_timeout;

    /* Figure out wtf the asm is doing here ?? */
    while(p->timeout--) {
        if (!(in(p->port) & p->clockin))	/* FIXME clock or data ? */
            return ps2_get(p);
    }
    return 0xFFFF;
}

