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
uint_fast8_t plt_param(unsigned char *p)
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
 *	Add all our free pages to the system. They are numbered in pairs as
 *	the system has some h/w logic to fake the 512K RAM/ROM enough for ROMWBW
 *	but not us.
 */
void pagemap_init(void)
{
	uint8_t i;
	pagemap_add(2);
	pagemap_add(3);
	/* 0/1 are the kernel */
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
