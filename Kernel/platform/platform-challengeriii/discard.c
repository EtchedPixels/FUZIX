#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>
#include <tinyide.h>
#include <ds1302.h>

uint16_t bankmask;	/* One bit per bank present bit 15 being system */

extern int strcmp(const char *, const char *);

/*
 *	Everything in this file ends up in discard which means the moment
 *	we try and execute init it gets blown away. That includes any
 *	variables declared here so beware!
 */

/*
 *	We get passed each kernel command line argument. if we return 1 then
 *	we claim it, if not it gets passed to init. It's perfectly acceptable
 *	to act on a match and return to also pass it to init if you need to.
 */
uint_fast8_t plt_param(unsigned char *p)
{
	return 0;
}

/*
 *	Set up our memory mappings. This is not needed for simple banked memory
 *	only more complex setups such as 16K paging.
 */
void map_init(void)
{
}

/*
 *	Add all the available pages to the list of pages we an use. If this
 *	is runtime dynamic check to make sure you don't add more than MAX_MAPS
 *	of them. On some machines with a lot of RAM the implementation turns
 *	the excess into a RAM disc
 *
 *	Maps are inverted 4 upper bits. 0F is th kernel.
 */
void pagemap_init(void)
{
	unsigned i = 1;
	unsigned m = 1;
	while(i < 15) {
		if (bankmask & m)
			pagemap_add(i);
		i++;
		m <<= 1;
	}
}

/*
 *	Called after interrupts are enabled in order to enumerate and set up
 *	any devices. In our case we set up the DS1302 for a time reference
 *	and then probe the IDE card.
 */

/* Disk PIA has the timer tick */
static volatile uint8_t *piab = (volatile uint8_t *) 0xC010;

void device_init(void)
{
	/* Timer */

	piab[3] = 0xB1;
	/* PIA for IDE at F80x */
	osihd_install();
#if 0
	ide_setup();
	ide_probe();
#endif
}
