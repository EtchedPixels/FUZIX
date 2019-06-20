#include <kernel.h>
#include <printf.h>
#include <blkdev.h>

extern uint8_t membanks;

uint16_t features;

void pagemap_init(void)
{
 int i;
 /* Up to 16 banks */
 kprintf("%d memory banks detected.\n", membanks);
 for (i = 0x81; i < 0x80 + membanks; i++)
  pagemap_add(i);
}

uint8_t platform_param(char *p)
{
    used(p);
    return 0;
}

/* Nothing to do for the map of init */
void map_init(void)
{
}
