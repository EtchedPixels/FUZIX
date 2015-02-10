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
    int i;
    /* 0/1/2 image, 3/4/5 kernel 6-19 apps */
    /* Don't add page 6 yet - it's the initial common at boot */
    for (i = 0x80 + 7; i < 0x80 + 20; i++)
        pagemap_add(i);
    /*
     * The kernel boots with 0x86 as the common, list it last here so it also
     * gets given to init as the kernel kicks off the init stub. init will then
     * exec preserving this common and all forks will be copies from it.
     */
    pagemap_add(0x86);
}

void map_init(void)
{
}
