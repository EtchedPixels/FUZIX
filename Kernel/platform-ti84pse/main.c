#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>

uaddr_t ramtop = PROGTOP;

void pagemap_init(void)
{
 int i;
 for (i = 1; i < 8; i++)
  pagemap_add(i);
}

void platform_idle(void)
{
}

void platform_interrupt(void)
{
}

void map_init(void)
{
}

uint8_t platform_param(char *p)
{
 used(p);
 return 0;
}

