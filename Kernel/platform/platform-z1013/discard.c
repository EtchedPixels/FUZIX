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

#define	PIO_C	0x01
#define CTC(x)	(0x3C + (x))

void device_init(void)
{
	out(PIO_C, 0xCF);	/* Mode 3 */
	out(PIO_C, 0x70);	/* MISO input, unused as input (so high Z) */
#ifdef CONFIG_PIO_TICK
	out(PIO_C, 0xF0);	/* interrupt vector F2, 0x5FF0 */
	out(PIO_C, 0x97);	/* interrupt on low, mask to follow */
	out(PIO_C,  0x08);		/* bit 3 has the 10Hz square wave */
#else
	out(PIO_C, 0x07);		/* No interrupt, no mask */
#endif
#ifdef CONFIG_K1520_SOUND	/* We care about the CTC not the sound */
	/* The timers run 0->1->2->3 chained and the clock input is 1.79/4 Mhz */
	out(CTC(0), 0x55);	/* counter, falling */
	out(CTC(0), 250);	/* time constant */
	out(CTC(1), 0xD5);	/* int, counter, falling */
	out(CTC(1), 179);	/* time constant */
	out(CTC(1), 0xF0);	/* Vector 0x5FF2 (channel 1) */
	out(CTC(2), 0x01);	/* Ensure they are off */
	out(CTC(3), 0x01);
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
