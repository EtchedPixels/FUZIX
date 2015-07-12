#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>


void platform_idle(void)
{
}

void do_beep(void)
{
}

/*
 Map handling: We have flexible paging. Each map table consists
 of a set of pages with the last page repeated to fill any holes.
 */

void pagemap_init(void)
{
    int i;
    /*  We have 64 8k pages for a CoCo3 so insert every other one
     *  into the kernel allocator map.
     */
    for (i = 8; i < 64; i+=2)
        pagemap_add(i);
    /* add common page last so init gets it */
    pagemap_add(6);
}

void map_init(void)
{
}


