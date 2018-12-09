#include <kernel.h>
#include <devtty.h>
#include <printf.h>

uaddr_t ramtop = PROGTOP;

void pagemap_init(void)
{
 int i;
 /* 1 << 0 is kernel */
 for (i = 1; i < 7; i++)
  pagemap_add(1 << i);
}

void platform_idle(void)
{
  /* halt ?? */
}

void platform_interrupt(void)
{
 tty_irq(1);
 tty_irq(2);
 tty_irq(3);
}

/* Nothing to do for the map of init */
void map_init(void)
{
}

uint8_t platform_param(char *p)
{
 used(p);
 return 0;
}
