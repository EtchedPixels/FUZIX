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
  tty_setup(3, 0);
}

void map_init(void)
{
}

uint8_t nbanks = 2;	/* Default 2 banks, unless we probe and find a
                           banked memory card */
extern uint8_t banktype;

/* The Hypermem select is not quite the same as 0 is the upper 64K
   original */
static void hyper_pagemap_init(void)
{
 uint8_t n;
 for (n = 0; n < nbanks; n++) {
  pagemap_add(n | 0x20);	/* Avoid getting a 0 value */
  pagemap_add(n | 0x80);
 }
}

void pagemap_init(void)
{
 uint8_t i = nbanks - 1;
 if (banktype == 2) {
  hyper_pagemap_init();
  return;
 }
 while (i) {
  pagemap_add(i);	/* Mode 3, U64K low 32K mapped as low 32K */
  pagemap_add(i|0x80);	/* Mode 3, U64K high 32K mapped as low 32K */
  i--;
 }
}
