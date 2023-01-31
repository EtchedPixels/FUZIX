/*
 *	Z80 PIO based GPIO ports on the Z80 PIO card
 */

#include <kernel.h>
#include <kdata.h>
#include <gpio.h>

#define NUM_GPIO	3

/* SC129 at 0x00 and console switches at 0xFF */

static struct gpio pioinfo[NUM_GPIO] = {
    {
        /* Console switches module lives at 0xFF read */
        0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, "CONSOLE"
    }
    /* If we don't have VD support then include support for the SC129
       at default 0 address */
    ,
    {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, "SC129R0"
    },
    {
        0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, "SC129W0"
    }
};

/* Track the actual data port as it's easier to mix types with that
   approach */
static uint8_t portmap[] = {
    0xFF, 0x00, 0x00
};

int gpio_ioctl(uarg_t request, char *data)
{
    struct gpio *p;
    uint8_t port;
    static struct gpioreq gr;

    /* No autodetect so always report the possible 24 */
    if (request == GPIOC_COUNT)
        return 24;

    if (uget(data, &gr, sizeof(struct gpioreq)) == -1)
        return -1;

    if (gr.pin >= NUM_GPIO * 8) {
        udata.u_error = ENODEV;
        return -1;
    }

    p = &pioinfo[gr.pin >> 3];
    port = portmap[gr.pin >> 3];

    switch(request) {
    case GPIOC_SETBYTE:
        p->wdata = gr.val;
        break;
    case GPIOC_SET:
        p->wdata |= gr.val;
        break;
    case GPIOC_CLR:
        p->wdata &= ~gr.val;
        break;
    case GPIOC_GETBYTE:
        return in(port) & ~p->wmask;
    case GPIOC_GETINFO:
        return uput(p, data, sizeof(struct gpio));
    case GPIOC_SETRW:	/* Fixed direction */
        /* Fall through */
    default:
        udata.u_error = ENOTTY;
        return -1;
    }
    out(port, p->wdata);
    return 0;
}
