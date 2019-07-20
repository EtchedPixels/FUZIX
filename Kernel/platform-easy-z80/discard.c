#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <ds1302.h>
#include <devide.h>
#include <blkdev.h>
#include <ppide.h>
#include <easy-z80.h>
#include "config.h"

/* Everything in here is discarded after init starts */

void init_hardware_c(void)
{
#ifdef CONFIG_VFD_TERM
	vfd_term_init();
#endif
	ramsize = 512;
	procmem = 512 - 80;
}

void pagemap_init(void)
{
	int i;

	/* RC2014 512/512K has RAM in the top 512 KiB of physical memory
	 * corresponding pages are 32-63 (page size is 16 KiB)
	 * Pages 32-34 are used by the kernel
	 * Page 35 is the common area for init
	 * Page 36 is the disk cache
	 */
	for (i = 32 + 5; i < 64; i++)
		pagemap_add(i);

	/* finally add the common area */
	pagemap_add(32 + 3);

	ds1302_init();

	kputs("Z80 SIO at 0x80.\n");
	kputs("Z80 CTC at 0x88.\n");
	if (ds1302_present)
		kputs("DS1302 detected at 0xC0.\n");
}

void map_init(void)
{
	/* Point the buffers into the 16-32K range that will be used by
	   the buffer remap. We offset by 0x4000 if need be in the access
	   functions when handling overlaps */
	bufptr bp = bufpool;
	uint8_t *p = (uint8_t *) 0x4000;
	while (bp < bufpool_end) {
		bp++->__bf_data = p;
		p += BLKSIZE;
	}
}

void device_init(void)
{
#ifdef CONFIG_IDE
	devide_init();
#ifdef CONFIG_PPIDE
	ppide_init();
#endif
#endif
}
