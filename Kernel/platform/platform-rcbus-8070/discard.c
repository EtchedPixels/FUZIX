#include <kernel.h>
#include <tinyide.h>

/*
 * Map handling: allocate 3 banks per process
 */

void pagemap_init(void)
{
    uint8_t i;
    /* 32-34 are kernel 35 is common so 36-63 are free */
    for (i = 36; i <= 63; i++)
        pagemap_add(i);
}

void map_init(void)
{
}

uint8_t plt_param(char *p)
{
    return 0;
}


void device_init(void)
{
	ide_probe();
        /* TODO: timer */
}
