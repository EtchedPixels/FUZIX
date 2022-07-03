#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <devsd.h>
#include <stdbool.h>
#include <blkdev.h>
#include "lib/dhara/map.h"
#include "lib/dhara/nand.h"
#include "globals.h"

static struct dhara_map dhara;
static const struct dhara_nand nand = 
{
	.log2_page_size = 9,
	.log2_ppb = 12 - 9,
    /* Number of blocks reported by mkftl (see also update-flash.sh). */
	.num_blocks = 496,
};

/*
 *	These cannot be disk cache becaause we may need to use them during an I/O and a call to tmpbuf
 *	could cause another I/O and recursion.
 */
static uint8_t journal_buf[512];
static uint8_t tmp_buf[512];

int dhara_nand_is_bad(const struct dhara_nand* n, dhara_block_t b)
{
	return 0;
}

void dhara_nand_mark_bad(const struct dhara_nand *n, dhara_block_t b) {}

int dhara_nand_is_free(const struct dhara_nand *n, dhara_page_t p)
{
	dhara_error_t err = DHARA_E_NONE;

	dhara_nand_read(&nand, p, 0, 512, tmp_buf, &err);
	if (err != DHARA_E_NONE)
		return 0;
	for (int i=0; i<512; i++)
		if (tmp_buf[i] != 0xff)
			return 0;
	return 1;
}

int dhara_nand_copy(const struct dhara_nand *n,
                    dhara_page_t src, dhara_page_t dst,
                    dhara_error_t *err)
{
	dhara_nand_read(&nand, src, 0, 512, tmp_buf, err);
	if (*err != DHARA_E_NONE)
		return -1;

	return dhara_nand_prog(&nand, dst, tmp_buf, err);
}

static uint_fast8_t transfer_cb(void)
{
	dhara_error_t err = DHARA_E_NONE;
	if (blk_op.is_read)
		dhara_map_read(&dhara, blk_op.lba, blk_op.addr, &err);
	else
		dhara_map_write(&dhara, blk_op.lba, blk_op.addr, &err);
	
	return (err == DHARA_E_NONE);
}

static int trim_cb(void)
{
	dhara_sector_t sector = blk_op.lba;
	if (sector < (nand.num_blocks << nand.log2_ppb))
		dhara_map_trim(&dhara, sector, NULL);
	return 0;
}

void flash_dev_init(void)
{
	blkdev_t* blk = blkdev_alloc();
	if (!blk)
		return;

	kprintf("NAND flash, ");
	dhara_map_init(&dhara, &nand, journal_buf, 128);
	dhara_error_t err = DHARA_E_NONE;
	dhara_map_resume(&dhara, &err);
	uint32_t lba = dhara_map_capacity(&dhara);
	kprintf("%dkB: ", lba / 2);
	
	blk->transfer = transfer_cb;
	blk->trim = trim_cb;
	blk->drive_lba_count = lba;
	blkdev_scan(blk, 0);
}

/* vim: sw=4 ts=4 et: */

