#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <tinyide.h>
#include <tinysd.h>
#include <devfdc765.h>
#include "config.h"

extern void rd_probe(void);
extern void sd_setup(void);
extern uint8_t fd765_present;
extern uint8_t *fd765_probe(void);

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
__sfr __at 0x3C ctc_0;
__sfr __at 0x3D ctc_1;
__sfr __at 0x3E ctc_2;
__sfr __at 0x3F ctc_3;

void device_init(void)
{
	pio_c = 0xCF;		/* Mode 3 */
	pio_c = 0x70;		/* MISO input, unused as input (so high Z) */
#ifdef CONFIG_PIO_TICK
	pio_c = 0xF0;		/* interrupt vector F2, 0x5FF0 */
	pio_c = 0x97;		/* interrupt on low, mask to follow */
	pio_c = 0x08;		/* bit 3 has the 10Hz square wave */
#else
	pio_c = 0x07;		/* No interrupt, no mask */
#endif
#ifdef CONFIG_K1520_SOUND	/* We care about the CTC not the sound */
	/* The timers run 0->1->2->3 chained and the clock input is 1.79/4 Mhz */
	ctc_0 = 0x55;		/* counter, falling */
	ctc_0 = 250;		/* time constant */
	ctc_1 = 0xD5;		/* int, counter, falling */
	ctc_1 = 179;		/* time constant */
	ctc_1 = 0xF0;		/* Vector 0x5FF2 (channel 1) */
	ctc_2 = 0x01;		/* Ensure they are off */
	ctc_3 = 0x01;
	/* Set up for 10Hz */
#endif
	rd_probe();
#ifdef CONFIG_TD_IDE
	ide_probe();
#endif
#ifdef CONFIG_TD_SD
	sd_setup();
	sd_probe();
#endif
#ifdef CONFIG_FDC765
	if (fd765_probe())
		fdc765_present = 3;
#endif
}
