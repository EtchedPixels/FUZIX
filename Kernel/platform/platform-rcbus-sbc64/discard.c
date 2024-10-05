#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <ds1302.h>
#include <tinyide.h>
#include <rcbus.h>
#include "config.h"

extern uint8_t kbank;

void map_init(void)
{
}

void pagemap_init(void)
{
  if (kbank == 3)
    kputs("Detected: Z80MB/SB64\n");
  else
    kputs("Detected: ZRCC\n");
  /* The high bits do nothing but it's a cheap way to avoid 0x00 */
  pagemap_add(0x12);
  pagemap_add(0x10);
  /* On the ZRCC our kernel is 1 and the common is 3, on the
     SBC64 our kernel is 3 and the common is 1 */

  ds1302_init();

  if (sio_present)
    kputs("Z80 SIO detected at 0x80.\n");
  if (sio1_present)
    kputs("Z80 SIO detected at 0x84.\n");
  if (ctc_present) {
    kputs("Z80 CTC detected at 0x88.\n");
    plt_tick_present = 1;
  }
  if (ds1302_present)
    kputs("DS1302 detected at 0xC0.\n");
}

void device_init(void)
{
	ide_probe();
}
