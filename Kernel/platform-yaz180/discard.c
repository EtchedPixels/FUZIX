#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include "config.h"
#include <z180.h>

void init_hardware_c(void)
{
    ramsize = 912;
    procmem = 848;
    /* zero out the initial bufpool */
    memset(bufpool, 0, (char*)bufpool_end - (char*)bufpool);
}

void pagemap_init(void)
{
    int i;

    /* YAZ180 has RAM from 0C000 to EFFFF
     * First 64K is used by the kernel. 
     * Each process gets the full 64K for now.
     * Page size is 4KB.
     * We don't do anything right now with C000-FFFF so that is
     * a good place to load any additional tools
     */
    for(i = 0x20; i < 0xF0; i+=0x10)
        pagemap_add(i);
}

void map_init(void)
{
    /* clone udata and stack into a regular process bank, return with common memory
       for the new process loaded */
    copy_and_map_proc(&init_process->p_page);
    /* kernel bank udata (0x300 bytes) is never used again -- could be reused? */
}

uint8_t plt_param(char *p)
{
    used(p);
    return 0;
}
