#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>
#include <devide.h>
#include <devsd.h>
#include "config.h"

extern void rd_probe(void);
extern void sd_setup(void);

void map_init(void)
{
#ifdef CONFIG_RD_SWAP
	uint_fast8_t i;
	for (i = 0; i < MAX_SWAPS; i++)
		swapmap_init(i);
#endif
	/* Adjust as this varies by video config option */
	ramsize = PROC_SIZE;
}

__sfr __at 0x01 pio_c;

void device_init(void)
{
	pio_c = 0xCF;		/* Mode 3 */
	pio_c = 0x70;		/* MISO input, unused as input (so high Z) */
#ifdef CONFIG_PIO_TICK
	pio_c = 0x02;		/* interrupt vector 2, 0x8002 */
	pio_c = 0x97;		/* interrupt on low, mask to follow */
	pio_c = 0x08;		/* bit 3 has the 10Hz square wave */
#else
	pio_c = 0x07;		/* No interrupt, no mask */
#endif
	rd_probe();
	sd_setup();
#ifdef CONFIG_IDE
	devide_init();
#endif
#ifdef CONFIG_SD
	devsd_init();
#endif
}
