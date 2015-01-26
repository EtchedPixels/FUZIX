/*
 *	This module manages a system with fixed sized banking where all but one
 *	area of memory is pinned and the other area can be set to multiple
 *	different banks.
 *
 *	FFFF		+-----------------------------------+
 *              	|          Common Memory            |
 *    MAPBASE+MAP_SIZE	+-----------------------------------+
 *	        	|          Banked Memory            |
 *      MAPBASE		+-----------------------------------+
 *			|          Common Memory	    |
 *	0000		+-----------------------------------+
 *
 *
 *	Other requirements:
 *	- 16bit address space (under 64K is fine, over is not) [Wants fixing]
 *
 *	Set:
 *	CONFIG_BANK_FIXED
 *	MAX_MAPS	to the number of banks available for user programs
 *	SWAPDEV		if using swap
 *	MAP_SIZE	the size of the bankable area
 *
 *	Most platforms using this module can use the standard lib/ tricks.s for
 *	their processor.
 *
 *	Bank numbers must not include 0 (0 is taken as swapped)
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>

#ifdef CONFIG_BANK_FIXED

/* Kernel is 0, apps 1,2,3 etc */
static unsigned char pfree[MAX_MAPS];
static unsigned char pfptr = 0;
static unsigned char pfmax;

void pagemap_add(uint8_t page)
{
	pfree[pfptr++] = page;
	pfmax = pfptr;
}

void pagemap_free(ptptr p)
{
	if (p->p_page == 0)
		panic("free0");
	pfree[pfptr++] = p->p_page;
}

int pagemap_alloc(ptptr p)
{
#ifdef SWAPDEV
	if (pfptr == 0) {
		swapneeded(p, 1);
	}
#endif
	if (pfptr == 0)
		return ENOMEM;
	p->p_page = pfree[--pfptr];
	return 0;
}

/* Realloc is trivial - we can't do anything useful */
int pagemap_realloc(uint16_t size)
{
	if (size > MAP_SIZE)
		return ENOMEM;
	return 0;
}

uint16_t pagemap_mem_used(void)
{
	return (pfmax - pfptr) * (MAP_SIZE >> 10);
}

#endif
