#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>

extern uint8_t kempston, kmouse;

uint8_t platform_param(char *p)
{
	return 0;
}

/* Nothing to do for the map of init */
void map_init(void)
{
}

void pagemap_init(void)
{
	uint8_t i;
	/* 8K pages that are above the kernel */
	/* FIXME: will depend upon size (<160 or < 224 for expanded)  */
	for (i = 24; i < 96; i++)
		pagemap_add(i);
	/* We can arguably also steal the memory wasted on the DivMMC
	   that we don't use FIXME */

	/* Low pages we can scavenge */
	/*
	    Reserved
		10-11: system screen (console 1)
		14-15: shadow screen (console 2)
	
	    We should probably reserve Layer 2 buffers as well at some
	    point in time
	*/		
	for (i = 0; i < 10; i++)
		pagemap_add(i);
	pagemap_add(12);
	pagemap_add(13);
	/* Common for init */
	pagemap_add(23);
}

void platform_copyright(void)
{
	kempston = 1;
	kmouse = 1;
}

