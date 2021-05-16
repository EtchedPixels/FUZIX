#include <kernel.h>
#include <devide.h>
#include <devsd.h>
#include <ds1302.h>

static const volatile uint8_t *iobase = (uint8_t *)IOBASE;

/*
 * Map handling: allocate 3 banks per process for now
 */

void pagemap_init(void)
{
    uint8_t i;
    for (i = 0x24; i <= 0x3D; i+= 3)
        pagemap_add(i);
    if (iobase[0x3F] & 3)
        panic("bad CONFIG");
}

void map_init(void)
{
}

uint8_t platform_param(char *p)
{
    return 0;
}


/*
 *	Do the device initialization
 */

void device_init(void)
{
	ds1302_init();
#ifdef CONFIG_IDE
	devide_init();
#endif
#ifdef CONFIG_SD
        devsd_init();
#endif
}
