#include <kernel.h>
#include <devide.h>

/*
 *	We have the following space
 *
 *	0-3F	8K pages of onboard DRAM. We use 38-3F for kernel right now
 *		but ought to change that. We want to end up with 0 as a kernel
 *		page as it's the magic 'not used' key
 *	40-7F	Only on the Genmod/Memex setup
 *	80-B7	Forwarded the TI99/4A P Box
 *	B8-BF	Peripheral card address block 0000-FFFF in 8K chunks
 *	C0-EB	SRAM expansion (Memex/Genod, sometimes 32K without)
 *	EC-EF	32K of onboard fast SRAM
 *
 *	F0-FF	EPROM
 *
 */

void pagemap_init(void)
{
    uint_fast8_t i;
    /* For now just shove in some pages we can use */
    for (i = 1; i < 0x38; i++)
        pagemap_add(i);
    pagemap_add(0x3F);	/* This will become the common of init as well */
}

void map_init(void)
{
}

uint_fast8_t plt_param(char *p)
{
    return 0;
}


/*
 *	We don't have a counter but a free running timer at system E clock
 *	rate (1.8432MHz) and compare register. We can reset the timer but
 *	that horks the serial port so we have to play comparison games or
 *	just go with the timer. Now as it happens the timer wraps 28 times
 *	a second to within 1%. Good enough for scheduling but will need minor
 *	compensation logic for the clock later TODO
 */

void device_init(void)
{
#ifdef CONFIG_IDE
	devide_init();
#endif
	/* TODO timer */
}
