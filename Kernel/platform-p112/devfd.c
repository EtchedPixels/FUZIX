/***************************************************************
   UZI (Unix Z80 Implementation) Kernel:  devflop.c
----------------------------------------------------------------
 Adapted from UZI By Doug Braun, and UZI280 by Stefan Nitschke
            Copyright (C) 1998 by Harold F. Bower
       Portions Copyright (C) 1995 by Stefan Nitschke
****************************************************************/
/* 2015-01-17 Will Sowerbutts: Ported from UZI-180 to Fuzix */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include "devfd.h"

/* functions implemented in devfd2.s */
extern int devfd_init(uint8_t minor);
extern int devfd_read(uint8_t minor);
extern int devfd_write(uint8_t minor);

/* variables in devfd2.s */
extern uint8_t devfd_track, devfd_sector, devfd_error;
extern char *devfd_buffer;

extern struct {
	uint8_t logged;
	uint8_t cbyte0;
	uint8_t cbyte1;
	uint8_t gap3;
	uint8_t spt;
	uint8_t sector1;
	uint8_t format;
	uint8_t spinup;
	uint8_t curtrk;
	uint8_t ncyl;
} devfd_dtbl[4];

static int fd_transfer(bool rwflag, uint8_t minor, uint8_t rawflag)
{
	uint8_t nblocks, blocks;
	uint16_t firstblk;
	uint16_t retc;
	irqflags_t irq;

	switch(rawflag){
		case 0:
			nblocks = 1;
			devfd_buffer = udata.u_buf->bf_data;
			firstblk = udata.u_buf->bf_blk;
			break;
		case 1:
			nblocks = udata.u_count >> 9;
			devfd_buffer = udata.u_base;
			firstblk = udata.u_offset >> BLKSHIFT;
			break;
#ifdef SWAPDEV
		case 2:
			nblocks = swapcnt >> 9;
			devfd_buffer = swapbase;
			firstblk = swapblk;
			break;
#endif
		default:
			goto failout;
	}

	devfd_track = firstblk / devfd_dtbl[minor].spt;
	devfd_sector = firstblk % devfd_dtbl[minor].spt; /* Base 0 Sect # */
	devfd_error = 0;

	if (devfd_track >= devfd_dtbl[minor].ncyl){
		goto failout;
	}

	blocks = nblocks;
	for (;;)
	{
		irq = di();
		if (rwflag)
			retc = devfd_read(minor);
		else
			retc = devfd_write(minor);
		irqrestore(irq);

		if (retc)
			break;

		if(--nblocks == 0)
			break;

		if (++devfd_sector > devfd_dtbl[minor].spt)
		{
			devfd_sector = 0;
			++devfd_track;
		}
		devfd_buffer += 128 << (devfd_dtbl[minor].format & 3);
	}

	if (devfd_error) {
		kprintf("fd_%s: error %d track %d sector %d\n",
				rwflag ? "read" : "write", devfd_error, devfd_track, devfd_sector);
		panic("fd_transfer");
	}

	if (retc) 
		goto failout;

	return blocks;
failout:
	udata.u_error = ENXIO;
	return -1;
}

int fd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
	flag; /* unused */
	return fd_transfer(true, minor, rawflag);
}

int fd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
	flag; /* unused */
	return fd_transfer(false, minor, rawflag);
}

int fd_open(uint8_t minor, uint16_t flags)
{
	flags; /* unused */

	if (devfd_init(minor)) {
		udata.u_error = ENXIO;
		return -1;
	}

	return 0;
}

int fd_close(uint8_t minor)
{
	devfd_dtbl[minor].logged = 0;	/* Mark Drive as logged out */
	return 0;
}
