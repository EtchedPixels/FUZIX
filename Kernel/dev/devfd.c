/****************************************************************
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
#include <fdc.h>
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

#define IBMPC35 0xAE    /* 10101110B HD,  DD, DS, 3.5",   512-byte Sctrs (1.44 MB) */
#define UZIHD3  0xAF    /* 10101111B HD,  DD, DS, 3.5",  1024-byte Sctrs (1.76 MB) */
#define IBMPC5  0xAA    /* 10101010B HD,  DD, DS, 5.25",  512-byte Sctrs (1.2 MB)  */
#define UZIHD5  0xAB    /* 10101011B HD,  DD, DS, 5.25", 1024-byte Sctrs (1.44 MB) */
#define DSQD35  0x2F    /* 00101111B MFM, DD, DS, 3.5",  1024-byte Sctrs (800 KB)  */
#define DSDD35  0x2E    /* 00101110B MFM, DD, DS, 3.5",   512-byte Sctrs (800 KB)  */
#define DSDD3	0x2E    /* 00101110B MFM, DD, DS, 3.5",   512-byte Sctrs (800 KB)  */
#define DSQD5   0x2B    /* 00101011B MFM, DD, DS, 5.25", 1024-byte Sctrs (800 KB)  */
#define DSDD5   0x2A    /* 00101010B MFM, DD, DS, 5.25",  512-byte Sctrs (800 KB)  */

struct devfd_dtbl {
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
    { 0, 0xCF, 1, 27, 18, 1, IBMPC35, 10, 0, 160 },
    { 0, 0xCF, 1, 27, 18, 1, IBMPC35, 10, 0, 160 },
    { 0, 0xCF, 1, 27, 18, 1, IBMPC35, 10, 0, 160 },
    { 0, 0xCF, 1, 27, 18, 1, IBMPC35, 10, 0, 160 },
#endif
};

#ifdef CONFIG_FLOPPY_NOHD
static uint8_t mode[4] = { 1, 1, 1, 1 };
#else
static uint8_t mode[4];
#endif

#define NUM_FDCMODES		7

static struct fdcinfo fdccap = {
	0,
	0,
	0,
	/* Our low level code is only tested with 512 byte sector right now */
	FDF_SD|FDF_DD|FDF_HD|FDF_DS|FDF_8INCH|FDF_SEC512,
	18,
	80,
	2,
	0,
};
	
static const struct fdcinfo fdcmode[NUM_FDCMODES] = {
	/* IBM PC 3.5" */
	{
		0,
		FDTYPE_PC144, /* 3.5" 1.44MB */
		0,
		FDF_HD|FDF_DS|FDF_SEC512,
		80,
		18,
		2,
		0
	},
	{
		1,
		FDTYPE_PC720, /* 3.5" 720K */
		0,
		FDF_DD|FDF_DS|FDF_SEC512,
		80,
		9,
		2,
		0
	},
	/* IBM PC 5.25" */
	{
		2,
		FDTYPE_PC12, /* 5.25" 1.2MB */
		0,
		FDF_HD|FDF_DS|FDF_SEC512,
		80,
		15,
		2,
		0
	},
	{
		3,
		FDTYPE_PC360, /* 5.25" 360K */
		0,
		FDF_DD|FDF_DS|FDF_SEC512,
		40,
		9,
		2,
		0
	},
	/* Amstrad 3" */
	{
		4,
		FDTYPE_AMS720, /* 3" 720K : looks like PC but different step rates etc */
		0,
		FDF_DD|FDF_DS|FDF_SEC512,
		80,
		9,
		2,
		0
	},
	{
		5,
		FDTYPE_AMS180, /* 3" 180K */
		0,
		FDF_DD|FDF_SEC512,
		40,
		9,
		1,
		0
	},
	{
		6,
		FDTYPE_PC180, 	/* 5.25" 180K */
		0,
		FDF_DD|FDF_SEC512,
		40,
		9,
		1,
		0
	},
	/* Should add some single density support or 8" ?? */
};

/* Settings for the configurations as the controller wants them */

static const struct devfd_dtbl devfd_modes[NUM_FDCMODES] = {
	/* 3.5" 3ms step */
	{ 0, 0xCF, 1, 27, 18, 1, IBMPC35, 10, 0, 160 },
	{ 0, 0xCF, 1, 27,  9, 1, DSDD35, 10, 0, 160 },
	/* 5.25 " */
	{ 0, 0xCF, 1, 27, 18, 1, IBMPC5, 10, 0, 160 },
	/* 360K drives step at 6ms */
    	{ 0, 0xAF, 1, 27,  9, 1, DSDD5, 10, 0, 80 },
	/* 3" 12ms step */
	{ 0, 0x4F, 1, 27,  9, 1, DSDD3, 10, 0, 40 },
	{ 0, 0x4F, 1, 27,  9, 1, DSDD3, 10, 0, 160 },
	/* 5.25" 360K singled side for data transfer */
	{ 0, 0xAF, 1, 27,  9, 1, DSDD3, 10, 0, 40 }
};
	
		

static int fd_transfer(bool rwflag, uint_fast8_t minor, uint_fast8_t rawflag)
{
	uint_fast8_t nblocks, blocks;
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

int fd_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
	flag; /* unused */
	return fd_transfer(true, minor, rawflag);
}

int fd_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
	flag; /* unused */
	return fd_transfer(false, minor, rawflag);
}

int fd_ioctl(uint_fast8_t minor, uarg_t request, char *data)
{
	uint8_t m;
	switch(request) {
	case FDIO_GETCAP:
		fdccap.mode = mode[minor];
		return uput(&fdccap, data, sizeof(struct fdcinfo));
	/* TODO: RESTORE, FMTTRK, SETSTEP */
	}
	m = ugetc(data);
	if (m >= NUM_FDCMODES) {
		udata.u_error = EINVAL;
		return -1;
	}
	switch(request) {
	case FDIO_GETMODE:
		return uput(fdcmode + m, data, sizeof(struct fdcinfo));
	case FDIO_SETMODE:
		mode[minor] = m;
		memcpy(devfd_dtbl + minor, devfd_modes + m, sizeof(struct devfd_dtbl));
		if (devfd_init(minor)) {
			udata.u_error = EIO;
			return -1;
		}
		return 0;
	}
	return -1;
}	

int fd_open(uint_fast8_t minor, uint16_t flags)
{
	flags; /* unused */

	if (devfd_init(minor) && !(flags & O_NDELAY)) {
		udata.u_error = ENXIO;
		return -1;
	}

	return 0;
}

int fd_close(uint_fast8_t minor)
{
	devfd_dtbl[minor].logged = 0;	/* Mark Drive as logged out */
	return 0;
}
