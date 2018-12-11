#include <kernel.h>
#include <devtty.h>
#include <printf.h>
#include <irq.h>

/* TODO: probe banks */
void pagemap_init(void)
{
 int i;
 /* 1 << 0 is kernel */
 for (i = 1; i < 7; i++)
  pagemap_add(1 << i);
}

/* Nothing to do for the map of init but we do set our vectors up here */
void map_init(void)
{
 if (request_irq(0xE7, uart0a_rx) |
 request_irq(0xEF, uart0a_txdone) |
 request_irq(0xF7, uart0a_timer4))
  panic("irqset");
 /* We need to claim these in case we set one off as they are at odd vectors
    as the base tu_uart is strapped for 8080 mode */
 if (
  request_irq(0xC7, spurious) |
  request_irq(0xCF, spurious) |
  request_irq(0xD7, spurious) |
  request_irq(0xDF, spurious) |
  request_irq(0xFF, spurious)
  )
  panic("irqset2");
}

uint8_t platform_param(char *p)
{
 used(p);
 return 0;
}
