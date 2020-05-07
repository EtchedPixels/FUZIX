#include <kernel.h>
#include <devide.h>

/* Onboard I/O */
static volatile uint8_t *cpuio = (volatile uint8_t *)0xF000;

/*
 * Map handling: allocate 4 banks per process
 */

void pagemap_init(void)
{
    uint8_t i;
    for (i = 36; i <= 63; i++)
        pagemap_add(i);
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
#ifdef CONFIG_IDE
	devide_init();
#endif
        /* Slowest RTI rate available */
	cpuio[0x26] |= 3;
	/* RTI interrupt enable */
	cpuio[0x24] |= 0x40;
}
