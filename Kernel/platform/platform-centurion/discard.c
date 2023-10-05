#include <kernel.h>

/* Onboard I/O */
static volatile uint8_t *cpuio = (volatile uint8_t *)0;

/*
 *	We don't know how the MMU works yet
 */

void pagemap_init(void)
{
    uint8_t i;
    /* Add the upper 192K  (7E/7F are the I/O space) */
    for (i = 0x20; i < 0x7E; ++i)
        pagemap_add(i);
}

uint_fast8_t plt_param(char *p)
{
    return 0;
}

void device_init(void)
{
    /* TODO: timers, disks etc */
}
