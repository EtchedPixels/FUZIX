#include <kernel.h>
#include <devhd.h>
#include <devtty.h>
#include <tty.h>
#include <kdata.h>
#include "devfd3.h"
#include "devgfx.h"
#include <devstringy.h>
#include "trs80.h"

void device_init(void)
{
#ifdef CONFIG_RTC
  /* Time of day clock */
  inittod();
#endif
  floppy_setup();
  hd_probe();
  gfx_init();
  tape_init();
}

void map_init(void)
{
}

uint8_t nbanks;

static void pagemap_init_supermem(void)
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

/* We have 128K available in the selector, of which 32K is taken up by
   a bank of kernel, leaving 3 user banks with 192K. For the LNW80 where
   there will be 64K in the base it's a different story and probably a
   different port ? */
static void pagemap_init_selector(void)
{
 int i = procmem / 32;
 if (i > MAX_MAPS)
  i = MAX_MAPS;
 while(i) {
  /* 32K bank i, map 0 (I/O in low 16K bank in top 32), banking on */
  pagemap_add((i << 4) + 8);
  i--;
 }
}

void pagemap_init(void)
{
 if (trs80_mapper == MAP_SELECTOR)
  pagemap_init_selector();
 else
  pagemap_init_supermem();
}
