/*
 *	Simple swap only system
 */

#include <kernel.h>
#include <timer.h>
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
<<<<<<< HEAD
  if (size >= ramtop)
=======
  if (size + (uint16_t)PROGBASE >= (uint16_t) ramtop)
>>>>>>> 23a5e496d3e31fef5c566130fcc3ab973c5ede4c
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
