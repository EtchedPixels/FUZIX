#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

/* The uarea is already synched to the stash which is written with the
   process */
uint8_t *swapout_prepare_uarea(ptptr p)
{
  p;
  return NULL;
}

/* The switchin code will move the uarea into the process itself, we just
   need to fix up the u_page pointer */
uint8_t *swapin_prepare_uarea(ptptr p)
{
  p;
  return NULL;
}

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
}

void map_init(void)
{
}
