#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include "config.h"
#include <z180.h>

void init_hardware_c(void)
{
    ramsize = 1024;
    procmem = 1024 - 64;
    /* zero out the initial bufpool */
    memset(bufpool, 0, (char*)bufpool_end - (char*)bufpool);
}

void pagemap_init(void)
{
    int i;

    /* 16 x 64K banks but 0x80000 is used by the kernel */
    for(i = 0x00; i < 0x100; i += 0x10)
        if (i != 0x80)	/* Kernel map */
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
