#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <tty.h>
#include <devices.h>
#include <devfd.h>
#include <devsys.h>
#include <devlpr.h>
#include <devtty.h>

void device_init(void)
{
  int i;
  /* Add 4 swaps (128K) to use the entire RAM drive */
  for (i = 0; i < MAX_SWAPS; i++)
    swapmap_add(i);
}
