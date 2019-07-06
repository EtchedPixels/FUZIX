#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>
#include <devfdc765.h>

void pagemap_init(void)
{
 uint_fast8_t i;
 uint_fast8_t m = 2;
 
 for (i = 1; i < 8; i++) {
  if (probe_bank(m) == 0) {
	pagemap_add(m);
	ramsize += 48;
	procmem += 48;
  }
  m <<= 1;
 }
 if (procmem < 96)
 	panic("insufficient memory");
}

/* Nothing to do for the map of init */
void map_init(void)
{
}

uint_fast8_t platform_param(char *p)
{
 used(p);
 return 0;
}
