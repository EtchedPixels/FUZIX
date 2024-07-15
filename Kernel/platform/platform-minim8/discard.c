#include <kernel.h>
#include <tinydisk.h>
#include <tinysd.h>

static const volatile uint8_t *iobase = (uint8_t *)IOBASE;

/*
 * Map handling: allocate 3 banks per process for now
 */

void pagemap_init(void)
{
    unsigned i = 0x32;		/* 01/00 is the kernel */
    while (i < 0x100) {
        pagemap_add(i);
        i += 0x22;		/* 32 54 76 98 BA DC FE */
    }
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
    sd_probe();
    netdev_init();
}
