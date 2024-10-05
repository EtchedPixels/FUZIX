#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>
#include "config.h"
#include <z180.h>
#include <ds1302.h>
#include <tty.h>

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

    /* RC2014 has RAM in the top 512K of physical memory. 
     * First 64K is used by the kernel. 
     * Each process gets the full 64K for now.
     * Page size is 4KB. */
    for(i = 0x90; i < (1024 >> 2); i += 0x10)
        pagemap_add(i);
#ifdef CONFIG_RTC_DS1302
    ds1302_init();
    if (ds1302_present)
        kprintf("DS1302 detected at 0x%2x.\n", rtc_port);
#endif
}

void map_init(void)
{
    /* clone udata and stack into a regular process bank, return with common memory
       for the new process loaded */
    copy_and_map_proc(&init_process->p_page);
    /* kernel bank udata (0x300 bytes) is never used again -- could be reused? */
}

static void turbo_on(void)
{
    kputs("Hold onto your hat...");
    /* Most boards use 55ns SRAM: that needs 2 wait states. 45ns would need
       1 but is rarer. Use max wait states for I/O for the moment */
    Z180_DCNTL |= 0xF0;		/* Force slow as possible, then mod back */
    Z180_DCNTL &= 0xBF;		/* 2 wait memory, 4 on I/O */
    Z180_RCR &= 0x7F;		/* No DRAM, kill refresh */
    Z180_CMR &= 0x7F;		/* Clock doubler off */
    if (Z180_CMR & 0x80)
        kputs("no clock doubler, 18.4MHz.\n");
    else {
        tty_setup(BOOT_TTY, 1);
        kputs("turbo engaged, 36.8MHz.\n");
        Z180_CMR |= 0x80;		/* Clock doubler on */
        Z180_CCR |= 0x80;		/* Clock divider off */
        turbo = 1;
    }
}

uint8_t plt_param(char *p)
{
    if (strcmp(p, "turbo") == 0) {
        turbo_on();
        return 1;
    }
    return 0;
}
