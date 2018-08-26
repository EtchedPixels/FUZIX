/***************************************************************
   UZI (Unix Z80 Implementation) Kernel:  devflop.c
----------------------------------------------------------------
 Adapted from UZI By Doug Braun, and UZI280 by Stefan Nitschke
            Copyright (C) 1998 by Harold F. Bower
       Portions Copyright (C) 1995 by Stefan Nitschke
****************************************************************/
/* 2015-01-17 Will Sowerbutts: Ported from UZI-180 to Fuzix */
/* Assumes 512-byte sectors, 3.5" 1.44 MB formatted disks   */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devfd.h>

/* functions implemented in devfd2.s */
extern int devfd_init(uint8_t minor);
extern int devfd_read(uint8_t minor);
extern int devfd_write(uint8_t minor);

/* variables in devfd2.s */
extern uint8_t devfd_track, devfd_sector, devfd_error, devfd_userbuf;
extern char *devfd_buffer;

/* D D D D D D D D               Format Byte
   7 6 5 4 3 2 1 0
   | | | | | | +-+----- Sector Size: 000=128, 001=256, 010=512, 011=1024 bytes
   | | | | +-+--------- Disk Size: 00=fixed disk, 01=8", 10=5.25", 11=3.5"
   | | | +------------- 0 = Normal 300 RPM MFM,    1 = "High-Density" Drive
   | | +--------------- 0 = Single-Sided,          1 = Double-Sided
   | +----------------- 0 = Double-Density,        1 = Single-Density
   +------------------- 0 = 250 kbps (normal MFM), 1 = 500 kbps (Hi-Density) */

#define IBMPC3  0xAE    /* 10101110B HD,  DD, DS, 3.5",   512-byte Sctrs (1.44 MB) */
#define UZIHD3  0xAF    /* 10101111B HD,  DD, DS, 3.5",  1024-byte Sctrs (1.76 MB) */
#define IBMPC5  0xAA    /* 10101010B HD,  DD, DS, 5.25",  512-byte Sctrs (1.2 MB)  */
#define UZIHD5  0xAB    /* 10101011B HD,  DD, DS, 5.25", 1024-byte Sctrs (1.44 MB) */
#define DSQD3   0x2F    /* 00101111B MFM, DD, DS, 3.5",  1024-byte Sctrs (800 KB)  */
#define DSDD3   0x2E    /* 00101110B MFM, DD, DS, 3.5",   512-byte Sctrs (800 KB)  */
#define DSQD5   0x2B    /* 00101011B MFM, DD, DS, 5.25", 1024-byte Sctrs (800 KB)  */
#define DSDD5   0x2A    /* 00101010B MFM, DD, DS, 5.25",  512-byte Sctrs (800 KB)  */

struct {
	uint8_t logged;	    /* logged (0xff), unlogged (0) */
	uint8_t cbyte0;	    /* bits 7-4: step rate (4ms), bits 3-0: HUT (240ms) */
	uint8_t cbyte1;	    /* head load time in 4ms steps (0=infinite) */
	uint8_t gap3;  	    /* gap3 (size 512 = 27, 1024 = 13) */
	uint8_t spt;   	    /* physical sectors per track */
	uint8_t sector1;    /* first sector number */
	uint8_t format;	    /* format byte */
	uint8_t spinup;	    /* spinup (1/20-secs) */
	uint8_t curtrk;     /* current tranck number */
	uint8_t ncyl;       /* number of cylinders x heads */
} devfd_dtbl[4] = {
#ifdef CONFIG_FLOPPY_NOHD
    /* 720K */
    { 0, 0xCF, 1, 27, 9, 1, DSDD3, 10, 0, 160 },
    { 0, 0xCF, 1, 27, 9, 1, DSDD3, 10, 0, 160 },
    { 0, 0xCF, 1, 27, 9, 1, DSDD3, 10, 0, 160 },
    { 0, 0xCF, 1, 27, 9, 1, DSDD3, 10, 0, 160 },
#else
    { 0, 0xCF, 1, 27, 18, 1, IBMPC3, 10, 0, 160 },
    { 0, 0xCF, 1, 27, 18, 1, IBMPC3, 10, 0, 160 },
    { 0, 0xCF, 1, 27, 18, 1, IBMPC3, 10, 0, 160 },
    { 0, 0xCF, 1, 27, 18, 1, IBMPC3, 10, 0, 160 },
#endif
};

static int fd_transfer(bool rwflag, uint8_t minor, uint8_t rawflag)
{
	uint8_t nblocks, blocks;
	uint16_t firstblk;
	uint16_t retc;
	irqflags_t irq;

	if (rawflag == 1)
		if (d_blkoff(9))
			return -1;

	/* Needs to learn to listen to swappage to support this */
	if (rawflag == 2) {
		udata.u_error = EIO;
		return -1;
	}

	if (rawflag)
		devfd_userbuf = 0xFF;

	firstblk = udata.u_block;
	devfd_track = firstblk / devfd_dtbl[minor].spt;
	devfd_sector = firstblk % devfd_dtbl[minor].spt; /* Base 0 Sect # */
	devfd_error = 0;

	if (devfd_track >= devfd_dtbl[minor].ncyl)
		goto failout;

	devfd_buffer = udata.u_dptr;
	blocks = udata.u_nblock;
	nblocks = blocks;

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
		goto failout;
	}

	if (retc) 
		goto failout;

	return blocks << BLKSHIFT;
failout:
	udata.u_error = EIO;
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
