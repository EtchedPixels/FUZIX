#include <kernel.h>
#include <printf.h>
#include <blkdev.h>

extern uint8_t membanks;

uint16_t features;

void pagemap_init(void)
{
 extern uint8_t kernel_map;
 int i;
 /* Up to 16 banks */
 kprintf("%d memory banks detected.\n", membanks);
 if (membanks < 3)
  panic("Insufficient memory.\n");
 kprintf("Kernel in bank %d.\n", kernel_map & 0x7F);
 for (i = 0x80; i < 0x80 + membanks; i++)
  if (i != kernel_map)
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
