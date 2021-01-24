#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <devsd.h>
#include <stdbool.h>
#include <blkdev.h>
#include "ftl.h"
#include "globals.h"

static uint_fast8_t transfer_cb(void)
{
	uint32_t logical = blk_op.lba / 7;
	int sector = blk_op.lba % 7;

	if (blk_op.is_read)
		ftl_read(logical, sector, blk_op.addr);
	else
		ftl_write(logical, sector, blk_op.addr);
	
	return 1;
}

void flash_dev_init(void)
{
	blkdev_t* blk = blkdev_alloc();
	if (!blk)
		return;

	kprintf("Scanning flash: ");
	uint32_t lba = ftl_init();
	kprintf(" %dkB\n", lba / 2);
	
	blk->transfer = transfer_cb;
	blk->drive_lba_count = lba;
	blkdev_scan(blk, 0);
}

