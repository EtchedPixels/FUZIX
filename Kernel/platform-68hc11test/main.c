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
 * Map handling: We have flexible paging. Each map table consists of a set of pages
 * with the last page repeated to fill any holes.
 */

void pagemap_init(void)
{
    int i;
    for (i = 0x01; i < 0x08; i++)
        pagemap_add(i);
}

void map_init(void)
{
}


uint8_t need_resched;
