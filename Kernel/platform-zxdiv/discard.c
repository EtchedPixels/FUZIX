#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

void pagemap_init(void)
{
#ifdef SWAPDEV
  /* Swap */
  swapmap_init(0);
  swapmap_init(1);
  swapmap_init(2);
#endif
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

void platform_copyright(void)
{
}

