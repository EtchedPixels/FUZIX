#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

uint8_t membanks;
uint8_t system_id;

void platform_idle(void)
{
}

void do_beep(void)
{
}

/*
 * Map handling: We have flexible paging. Each map table consists of a set of pages
 * with the last page repeated to fill any holes.
 */

void pagemap_init(void)
{
	int i;
	/* map bank 1 last for init, leave 0 for kernel */
	for (i = membanks - 1; i > 0; i--)
		pagemap_add(i);

#ifdef SWAPDEV
	for (i = 0; i < MAX_SWAPS; i++)
		swapmap_add(i);
#endif
}

void map_init(void)
{
}
