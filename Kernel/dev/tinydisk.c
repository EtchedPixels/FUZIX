/*
 *	A minimal SD implementation for tiny machines
 *	Assumes
 *	- Firmware initialized the device
 *	- Only supports primary partitions
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#define _TINYDISK_PRIVATE
#include <tinydisk.h>

/* Used by the asm helpers */
uint8_t td_page;
uint8_t td_raw;
uint32_t td_lba[CONFIG_TD_NUM][MAX_PART + 1];
td_xfer td_op[CONFIG_TD_NUM];

static int td_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
	uint8_t dev = minor >> 4;
	uint16_t ct = 0;
	uint8_t *dptr;
	uint16_t nblock;
	uint32_t lba;

	minor &= 0x0F;

	td_page = 0;
	td_raw = rawflag;
	if (rawflag == 1) {
		if (d_blkoff(BLKSHIFT))
			return -1;
		td_page = udata.u_page;	/* User space */
	}
#if defined(SWAPDEV) || defined(PAGEDEV)
	else if (rawflag == 2)
		td_page = swappage;
#else
	else if (rawflag == 2)
		goto error;
#endif

	lba = udata.u_block;
	if (minor) {
		if (minor < MAX_PART && td_lba[dev][minor])
			lba += td_lba[dev][minor];
		else
			goto fail;
	}

	dptr = udata.u_dptr;
	nblock = udata.u_nblock;

	/* Here be dragons. In the swap case we will load over udata so watch
	   we avoid udata. values */
	while (ct < nblock) {
		if (td_op[dev] (dev, is_read, lba, dptr) == 0)
			goto error;
		ct++;
		dptr += 512;
		lba++;
	}
	return ct << 9;
error:
	kprintf("hd%c: I/O error\n", dev + 'a');
fail:
	udata.u_error = EIO;
	return -1;
}

int td_open(uint_fast8_t minor, uint16_t flag)
{
	uint8_t dev = minor >> 4;
	minor &= 0x0F;
	if (dev > CONFIG_TD_NUM || minor > MAX_PART || td_op[dev] == NULL) {
		udata.u_error = ENODEV;
		return -1;
	}
	return 0;
}

int td_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
	return td_transfer(minor, true, rawflag);
}

int td_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
	return td_transfer(minor, false, rawflag);

}

/* TODO */
int td_ioctl(uint_fast8_t minor, uarg_t request, char *data)
{
	udata.u_error = ENOTTY;
	return -1;
}
