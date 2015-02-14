#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <z180.h>
#include "config.h"
#ifdef CONFIG_P112_FLOPPY
#include "devfd.h"
#endif

void pagemap_init(void)
{
    int i;

    /* P112 has RAM across the full physical 1MB address space
     * First 64K is used by the kernel. 
     * Each process gets the full 64K for now.
     * Page size is 4KB. */
    for(i = 0x10; i < 0x100; i+=0x10){
        pagemap_add(i);
    }
}

void map_init(void)
{
    /* clone udata and stack into a regular process bank, return with common memory
       for the new process loaded */
    copy_and_map_process(&init_process->p_page);
    /* kernel bank udata (0x300 bytes) is never used again -- could be reused? */
}
