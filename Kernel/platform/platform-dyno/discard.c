#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include "config.h"
#include <z180.h>
#include <bq4845.h>

void init_hardware_c(void)
{
    ramsize = 512;
    procmem = 512 - 64;
    /* zero out the initial bufpool */
    memset(bufpool, 0, (char*)bufpool_end - (char*)bufpool);
}

void pagemap_init(void)
{
    int i;

    /* Low 512K ROM High 512K RAM */
    for(i = 0x90; i < (1024 >> 2); i += 0x10)
        pagemap_add(i);
    bq4845_init();
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
