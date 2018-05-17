#include <kernel.h>
#include <devhd.h>
#include <devtty.h>
#include <tty.h>

void device_init(void)
{
  vtbuf_init();
#ifdef CONFIG_RTC
  /* Time of day clock */
  inittod();
#endif
  hd_probe();
  tty_setup(3);
}

void map_init(void)
{
}

uint8_t nbanks = 2;	/* Default 2 banks, unless port 94 probe updates */

void pagemap_init(void)
{
#ifdef CONFIG_MAP94
 int i = nbanks - 1;
 while (i) {
  pagemap_add(i);	/* Mode 3, U64K low 32K mapped as low 32K */
  pagemap_add(i|0x80);	/* Mode 3, U64K high 32K mapped as low 32K */
  i--;
 }
#else
 pagemap_add(0x63);	/* Mode 3, U64K low 32K mapped as low 32K */
 pagemap_add(0x73);	/* Mode 3, U64K high 32K mapped as low 32K */
#endif
}
