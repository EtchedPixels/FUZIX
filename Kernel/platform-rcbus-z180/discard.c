#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <tinyide.h>
#include <tinysd.h>
#include "config.h"
#include <z180.h>
#include <ds1302.h>
#include <netdev.h>
#include <tty.h>
#include "rcbus-z180.h"

static uint8_t has_1mb;	/* additional 512K RAM located in U2 socket */

void init_hardware_c(void)
{
	/* Check for SC126 1MB mod */
	if (systype == 10)
		has_1mb = detect_1mb();

	if (has_1mb) {
		ramsize = 1024;
		procmem = 1024 - 64;
	} else {
		ramsize = 512;
		procmem = 512 - 64;
	}
	/* zero out the initial bufpool */
	memset(bufpool, 0, (char *) bufpool_end - (char *) bufpool);
	/* FIXME add a platform param for tuning wait states for memory
	   I/O and DMA (and one for the CF adapter of its own ?) */
}

void pagemap_init(void)
{
	int i;

	/* RC2014 has RAM in the top 512K of physical memory. 
	 * First 64K is used by the kernel. 
	 * Each process gets the full 64K for now.
	 * Set the low bit on the map indexes so that we can index page 0
	 * without confusing it with swap.
	 * Page size is 4KB. */
	for (i = 0x91; i < (1024 >> 2); i += 0x10)
		pagemap_add(i);
	/* Modded SC126 may have additional 512K starting at 0. */
	if (has_1mb)
		for (i = 0x01; i < (512 >> 2); i += 0x10)
			pagemap_add(i);
	ds1302_init();
	if (ds1302_present)
		kputs("DS1302 detected at 0x0C.\n");
}

void map_init(void)
{
	/* clone udata and stack into a regular process bank, return with common memory
	   for the new process loaded */
	copy_and_map_process(&init_process->p_page);
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
#ifdef CONFIG_NET
	if (strcmp(p, "wiznet") == 0 && systype != 10) {
		netdev_init();
		return 1;
	}
#endif
	if (strcmp(p, "turbo") == 0) {
		turbo_on();
		return 1;
	}
	return 0;
}

void device_init(void)
{
	ide_probe();
	if (systype == 10) {	/* Has SD glue */
		sd_init();
#ifdef CONFIG_NET
		netdev_init();
#endif
	}
}
