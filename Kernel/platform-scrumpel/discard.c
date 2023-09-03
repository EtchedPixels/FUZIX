#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>
#include "config.h"
#include <z180.h>
#include <ds1302.h>

extern int strcmp(void *, void *);

/* This is only used before discarding */
uint8_t has_sd = 0;

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

    /* Scrumpel has RAM in the low 512K of physical memory. 
     * First 64K is used by the kernel. 
     * Each process gets the full 64K for now.
     * Page size is 4KB. */
    for(i = 0x10; i < (512 >> 2); i += 0x10)
        pagemap_add(i);
#if 0        
    ds1302_init();
    if (ds1302_present)
        kputs("DS1302 detected at 0xC0.\n");
#endif        
}

void map_init(void)
{
	/* clone udata and stack into a regular process bank, return with
	   common memory for the new process loaded */
	copy_and_map_proc(&init_process->p_page);
	/* kernel bank udata (0x300 bytes) is never used again
	   -- could be reused? */
}

uint8_t plt_param(char *p)
{
	if (strcmp(p, "sd") == 0) {
		has_sd = 1;
		return 1;
	}
	return 0;
}

