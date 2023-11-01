#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <tinyide.h>

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
 *	The mapping can be logical numbers 1-n, or can be physical values to
 *	write into registers. Whatever works best. The 0 value is however
 *	reserved throughout to indicate kernel mode in calls, and also
 *	to mean swapped out for processes. If your bank 0 is user space you
 *	might need to just dec/inc before using it in an I/O port or similar
 *	to avoid confusion.
 *
 *	Kernel in bank 0, user in banks 2-15, high 32K is bank 1
 */
void pagemap_init(void)
{
	uint8_t i;
	for (i = 2; i <= 15; i++)
		pagemap_add(i);
}

/*
 *	Called after interrupts are enabled in order to enumerate and set up
 *	any devices.
 */

void device_init(void)
{
	ide_probe();
}
