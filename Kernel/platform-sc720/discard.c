#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <tinydisk.h>
#include <tinyide.h>
#include <ds1302.h>

extern uint8_t ctc_present;
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
uint8_t plt_param(unsigned char *p)
{
#if 0
	/* Need to add to the SIO driver TODO */
	if (strcmp(p, "msr") == 0) {
		timermsr = 1;
		plt_tick_present = 1;
		return 1;
        }
#endif        
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
 *	The mapping can be logical numbers 1-n, or can be physical values to
 *	write into registers. Whatever works best. The 0 value is however
 *	reserved throughout to indicate kernel mode in calls, and also
 *	to mean swapped out for processes. If your bank 0 is user space you
 *	might need to just dec/inc before using it in an I/O port or similar
 *	to avoid confusion.
 *
 *	Kernel in bank 0, user in banks 1-14, high 32K is bank 15
 *	With 128K the high is bank 3. The main code continues using 14/15 for
 *	the kernel and high pages but they wrap so there is no problem.
 *
 *	The pages are actually 0-15, but the top bit does nothing on the
 *	SBCv2. However by setting this bit we magically just work on the
 *	RB-MBC platform as well.
 */
void pagemap_init(void)
{
	uint8_t i;
	/* Kernel and top 32K are the upper 2 32K banks */
	for (i = 0x20; i < 0x3C; i+= 2)
		pagemap_add(i);
}

/*
 *	Called after interrupts are enabled in order to enumerate and set up
 *	any devices. In our case we set up the SIO and then probe the CF.
 */

void device_init(void)
{
	/* Probe the timers first as IDE needs a timer of some form */
	/* DS1302 can be at 0xC0 or 0x0C */
	if (ds1302_init() == 0) {
		rtc_port = 0x0C;
		if (ds1302_init() == 0 && ctc_present == 0)
			panic("no timer");
	}
	ide_probe();
	sock_init();
}
