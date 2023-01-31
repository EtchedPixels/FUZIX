#include <kernel.h>
#include <devide.h>

/* Onboard I/O */
static volatile uint8_t *cpuio = (volatile uint8_t *)0;

/*
 * Map handling: allocate 3 banks per process
 */

void pagemap_init(void)
{
    uint_fast8_t i;
    for (i = 36; i < 63; i++)
        pagemap_add(i);
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
