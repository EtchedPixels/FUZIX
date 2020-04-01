#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <ds1302.h>
#include <devide.h>
#include <blkdev.h>
#include <rc2014.h>
#include "config.h"

void map_init(void)
{
}

void pagemap_init(void)
{
  /* The high bits do nothing but it's a cheap way to avoid 0x00 */
  pagemap_add(0x12);
  pagemap_add(0x10);

  ds1302_init();

  if (sio_present)
    kputs("Z80 SIO detected at 0x80.\n");
  if (sio1_present)
    kputs("Z80 SIO detected at 0x84.\n");
  if (ctc_present) {
    kputs("Z80 CTC detected at 0x88.\n");
    platform_tick_present = 1;
  }
  if (ds1302_present)
    kputs("DS1302 detected at 0xC0.\n");
}

void device_init(void)
{
#ifdef CONFIG_IDE
	devide_init();
#endif
}
