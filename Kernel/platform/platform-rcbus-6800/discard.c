#include <kernel.h>
#include <tinyide.h>

/*
 * Map handling: addd all the spare 16K pages
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

uint_fast8_t plt_param(char *p)
{
    return 0;
}

void device_init(void)
{
    ide_probe();
}
