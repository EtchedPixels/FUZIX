#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include "config.h"
#include <z180.h>

void init_hardware_c(void)
{
    ramsize = 512;
    procmem = 512 - 64 - (DEV_RD_RAM_PAGES<<2);
    /* zero out the initial bufpool */
    memset(bufpool, 0, (char*)bufpool_end - (char*)bufpool);
}

void pagemap_init(void)
{
    int i;

    /* N8VEM SBC Mark IV has RAM in the top 512K of physical memory. 
     * First 64K is used by the kernel. 
     * Each process gets the full 64K for now.
     * Page size is 4KB. */
    for(i = 0x90; i < ((1024 - (DEV_RD_RAM_PAGES<<2))>>2); i+=0x10)
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
