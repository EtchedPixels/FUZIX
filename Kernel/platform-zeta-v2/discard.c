#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include "config.h"

void pagemap_init(void)
{
    int i;

    /* ZETA SBC V2 has RAM in the top 512 KiB of physical memory
     * corresponding pages are 32-63 (page size is 16 KiB)
     * Pages 32-34 are used by the kernel
     * Page 35 is the common area
     * Pages starting from DEV_RD_START are used by RAM disk
     */
    for (i = 32 + 4; i < DEV_RD_START; i++)
        pagemap_add(i);

    /* finally add the common area */
    pagemap_add(32 + 3);
}

void map_init(void)
{
}
