#include <kernel.h>
#include <devhd.h>
#include <devtty.h>
#include <tty.h>
#include <kdata.h>

void device_init(void)
{
#ifdef CONFIG_RTC
  /* Time of day clock */
  inittod();
#endif
  hd_probe();
}

void map_init(void)
{
}

uint8_t nbanks = 2;	/* Default 2 banks, unless port 94 probe updates */

void pagemap_init(void)
{
 int i = procmem / 32;	/* How many banks do we have */
 if (i > MAX_MAPS)
  i = MAX_MAPS;
 i += 2;		/* Banks 0/1 are the kernel and not included */
 while (i > 1) {
  pagemap_add(i);
  i--;
 }
}
