/*
    ZX Spectrum floppy drive driver for Betadisk-128 interface.

    Betadisk is based on wd1793 chip, but all its ports are hidden
    and only become available when an opcode fetch is performed in the ROM
    area 0x3D00..0x3DFF. This forces us to use TR-DOS ROM procedures
    instead of direct wd1793 access.

    Idea and initial version by b2m @ http://zx-pk.ru
*/

#include <kernel.h>
#include <kdata.h>

#include "betadisk.h"

int betadisk_open(uint8_t minor, uint16_t flag)
{
	flag;
	if(minor != 0) {
		udata.u_error = ENODEV;
		return -1;
	}
	return 0;
}

static int betadisk_transfer(bool is_read, uint8_t rawflag)
{
	blkno_t block;

	if (rawflag != 0)
		return 0;

	block = udata.u_buf->bf_blk<<1;

	/* Read only for now */
	if (!is_read)
		return 1;

	betadisk_seek_internal(block>>4);
	block &= 15;
	betadisk_read_internal(block, udata.u_buf->bf_data);
	betadisk_read_internal(block+1, udata.u_buf->bf_data+256);
	return 1;
}

int betadisk_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
	flag;minor;
	return betadisk_transfer(true, rawflag);
}

int betadisk_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;minor;
    return betadisk_transfer(false, rawflag);
}

