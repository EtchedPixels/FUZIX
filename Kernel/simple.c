/*
 *	This module manages a system with no user banking.
 *
 *	User memory lies between PROGBASE and PROGTOP which does not overlap
 *	common memory. The kernel may be banked over user memory areas if needed.
 *
 *	All task switching occurs by swapping the existing process out to storage
 *	and reading in the new one. This can be done for either single tasking or
 *	(with a hard disk) multitasking.
 *
 *	Other requirements:
 *	- 16bit address space (FIXME: should be made 32bit clean)
 *
 *	Set:
 *	CONFIG_SWAP_ONLY
 *
 *	Zero is used as swapped, 1 is used as in memory.
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>

#ifdef CONFIG_SWAP_ONLY

void pagemap_free(ptptr p)
{
  p->p_page = 0;
}

int pagemap_alloc(ptptr p)
{
  p->p_page = 1;
  return 0;
}

int pagemap_realloc(uint16_t size)
{
  if (size >= ramtop)
    return ENOMEM;
  return 0;
}

uint16_t pagemap_mem_used(void)
{
  return (PROGTOP - PROGBASE) >> 10;
}

void pagemap_init(void)
{
}

#endif
