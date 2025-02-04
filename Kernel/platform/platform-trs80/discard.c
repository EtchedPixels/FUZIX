#include <kernel.h>
#include <printf.h>
#include <devhd.h>
#include <devtty.h>
#include <tty.h>

__sfr __at 0x80 gfx_x;
__sfr __at 0x81 gfx_y;
__sfr __at 0x82 gfx_data;
__sfr __at 0x83 gfx_ctrl;
__sfr __at 0x8C gfx_xpan;
__sfr __at 0x8D gfx_ypan;
__sfr __at 0x8E gfx_xor;

/* There are two possible card types. The Tandy version is a 640x240 card with
   hardware panning. The Micro Grafyx is very similar but has no panning and
   has a 512 x 192 mode to match the model 3 modes

   The control port differs as follows
   Tandy
   0: set to turn graphics on, clear for alpha
   1: set to turn on wait states (no flicker on update)

   Micrografyx
   0: set to turn on graphics
   1: if graphics set to turn off text (otherwie xored)

   Unique to Tandy are
   0x8C: X pan byte value 0-127
   0x8D: Y pan 0-255
   0x8E: 1 = xor text and graphics, 0 = graphics only (other bits unknown)
*/

static uint_fast8_t probe_gfx(void)
{
 kputs("Probing gfx\n");
 gfx_x = 1;
 gfx_y = 1;
 gfx_ctrl = 0xA0;	/* Graphics off, inc X on read/write */
 gfx_data = 0x55;
 gfx_data = 0xAA;
 gfx_x = 1;
 if (gfx_data != 0x55) { 
  kputs("none\n") ; 
  return 0;
 }
 if (gfx_data != 0xAA) { 
  kputs("none\n") ; 
  return 0;
 }
 /* We have either a TRS80 or a clone card present */
 gfx_x = 0x7F;
 gfx_y = 0xFF;
 gfx_ctrl = 0xF0; 		/* No inc of X or Y */
 gfx_data = 0xA5;
 if (gfx_data != 0xA5) { 	/* this may need to be forced for Grafyx clone */
  kputs("uLabs Grafyx (original)\n") ; 	
  return 2;
 }
 kputs("Tandy (assumed) or Grafyx clone (unhandled)\n") ; 
 return 1;
}

void device_init(void)
{
  gfxtype = probe_gfx();
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
