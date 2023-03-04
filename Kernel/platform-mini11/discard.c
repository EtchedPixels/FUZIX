#include <kernel.h>
#include <tinydisk.h>
#include <tinysd.h>

static const volatile uint8_t *iobase = (uint8_t *)IOBASE;

/*
 * Map handling: allocate 3 banks per process for now
 */

void pagemap_init(void)
{
    /* FIXME: page numbering */
    uint8_t i;
    for (i = 32 + 4; i < 64; i++)
        pagemap_add(i);
    /* The common page must be the first one allocated so last added */
    pagemap_add(32 + 3);
    if (iobase[0x3F] & 3)
        panic("bad CONFIG");
}

void map_init(void)
{
}

uint8_t plt_param(char *p)
{
    return 0;
}


/*
 *	Do the device initialization
 */

void device_init(void)
{
    uint8_t t = sd_init();
    if (t == 0)
        return;
    if (t & CT_BLOCK)
        sd_shift[0] = 0;
    else
        sd_shift[0] = 9;
    td_register(sd_xfer, 1);
}
