/*
 *	Driver for the GM809/829/849/849A floppy controllers
 *	(The SASI/SCSI has its own driver)
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <nascom.h>
#include <devgm8x9.h>

#define gm8x9_cmd 0xE0
#define gm8x9_status 0xE0
#define gm8x9_track 0xE1
#define gm8x9_sector 0xE2
#define gm8x9_data 0xE3
#define gm8x9_ctrl 0xE4
#define gm8x9_type 0xE5

#define SIDE		1
#define DDENS		2
#define HDENS		4
#define EIGHTINCH	8

static uint8_t gm8x9_cursel;
uint8_t gm8x9_steprate;
static uint8_t drive_last = 0xFF, flags_last;

/* IBM3740 skew table */
static uint8_t skew_3740[] = {
	1, 7, 13, 19, 25, 5, 11, 17, 23, 3, 9, 15, 21, 2, 8, 14, 20, 26, 6, 12, 18, 24, 4, 10, 16, 22
};

/* Skewed at format level */
static uint8_t skew_hard[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19.20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};

struct gmfd gmfd_drives[MAX_GMFD];

/*
 *	Calculate the drive control value and if need be update it
 */
static uint_fast8_t gm8x9_select(uint_fast8_t drive, uint_fast8_t flags)
{
	uint_fast8_t ret = 1;

	if (drive_last == drive && flags_last == flags)
		return 0;

	if (drive_last != drive) {
		if (drive_last != 0xFF)
			gmfd_drives[drive_last].track = in(gm8x9_track);
		drive_last = drive;
		out(gm8x9_track, gmfd_drives[drive].track);
		gm8x9_steprate = gmfd_drives[drive].steprate;
		ret = 2;
	}

	flags_last = flags;


	if (in(gm8x9_type) & 0x80) {
		/* 809 / 829 */
		drive = 1 << drive;
		if (flags & SIDE)
			drive |= 0/*?FIXME?*/;
	} else {
		if (flags & HDENS)
			drive |= 0xB0;
		if (flags & SIDE)
			drive |= 0x08;
	}
	if (flags & DDENS)
		drive |= 0x10;
	if (flags & EIGHTINCH)
		drive |= 0x20;
	gm8x9_cursel = drive;
	out(gm8x9_ctrl,drive);
	return ret;
}

/*
 *	We only support normal block I/O for the moment. We do need to
 *	add swapping!
 */

static int gm8x9_transfer(uint_fast8_t minor, bool is_read, uint_fast8_t rawflag)
{
	int ct = 0;
	int tries;
	uint_fast8_t err = 0;
	uint_fast8_t side, sector, track;
	irqflags_t irqflags;
	register struct gmfd *fd = gmfd_drives + minor;

	if (rawflag == 2)
		goto bad2;

	/* Translate everything into physical sectors. d_blkoff does the work
	   for raw I/O we do it for normal block I/O */
	if (rawflag) {
		io_page = udata.u_page;
		if (d_blkoff(fd->bs))
			return -1;
	} else {
		io_page = 0;
		udata.u_nblock <<= fd->bs;
		udata.u_block <<= fd->bs;
	}

	/* Loop through each logical sector translating it into a head/track/sector
	   and then attempting to do the I/O a few times */
	while (ct < udata.u_nblock) {
		side = 0;
		sector = fd->skewtab[udata.u_block % fd->spt];
		track = udata.u_block / fd->spt;
		if (sector > fd->ds) {
			sector -= fd->ds;
			side = SIDE;
		}
		/* Set the drive parameters, also pokes the motor */
		gm8x9_select(minor, fd->dens | side);
		/* TODO - any delays ?? */

		/* Make multiple attempts to get the data. If it keeps failing try
		   restoring the head and seeking in order to re-align */
		for (tries = 0; tries < 5; tries++) {
			/* Try to get the requested track */
			if (in(gm8x9_track) != track) {
				if ((err = gm8x9_seek(track))) {
					gm8x9_restore();
					continue;
				}
			}
			/* The timing on these is too tight to do with interrupts on */
			irqflags = di();
			if (is_read)
				err = gm8x9_ioread(udata.u_dptr);
			else
				err = gm8x9_iowrite(udata.u_dptr);
			irqrestore(irqflags);

			/* It worked - exit then inner retry loop and move on */
			if (err == 0)
				break;
			/* Force a head seek */
			if (tries > 1)
				gm8x9_restore();
		}
		if (tries == 5)
			goto bad;
		/* Move on a sector */
		udata.u_block++;
		udata.u_dptr += fd->ss;
		ct++;
	}

	/* Data read in bytes */
	return udata.u_nblock << (9 - fd->bs);
      bad:
	kprintf("fd%d: error %x\n", minor, err);
      bad2:
	udata.u_error = EIO;
	return -1;
}

uint_fast8_t gm8x9_density(uint_fast8_t minor, uint_fast8_t flags)
{
	flags &= EIGHTINCH;
	/* Try double density */
	gm8x9_select(minor, DDENS | flags);
	if (gm8x9_restore_test() == 0)
		return DDENS | flags;
	/* Try single density */
	gm8x9_select(minor, flags);
	if (gm8x9_restore_test() == 0)
		return flags;
	/* We can only do HD with 5.25/3.5/3 inch media on an 849 or 849A */
	if ((gm8x9_type & 0x80) || (flags & EIGHTINCH))
		return 255;
	gm8x9_select(minor, HDENS | flags);
	if (gm8x9_restore_test() == 0)
		return flags | HDENS;
	/* Nothing worked */
	return 255;
}

int gm8x9_open(uint_fast8_t minor, uint16_t flag)
{
	uint_fast8_t den;
	register struct gmfd *d = gmfd_drives + minor;

	flag;
	if (((gm8x9_type & 0x80) && minor > 4) || minor > MAX_GMFD) {
		udata.u_error = ENODEV;
		return -1;
	}
	if ((den = gm8x9_density(minor, d->dens)) == 255 && !(flag & O_NDELAY)) {
		udata.u_error = -EIO;
		return -1;
	}
	/* FIXME: how to detect double sided ? */
	d->dens = den;

	/* Default media types need to add switching ioctls yet */
	/* Once we also have the media geometry info in the superblock
	   it'll get a *lot* easier */
	/* Also need to add soft skewing ioctl */
	d->bs = 0;
	d->ss = 512;
	d->ds = 0;
	memcpy(d->skewtab, skew_hard, MAX_SKEW);
	switch (den) {
	case 0:
		/* 18 spt 128bps double sided */
		d->spt = 36;
		d->bs = 2;
		break;
	case EIGHTINCH:
		/* IBM3740: Classic CP/M SS/SD 77 track 26 tps 128bps */
		d->spt = 26;
		d->ds = 255;
		memcpy(d->skewtab, skew_3740, MAX_SKEW);
		break;
	case DDENS:
	case 255:
		/* IBM PC style 5.25" double density is 18/9 but Nascom like many
		   other systems use 20/10 */
		d->spt = 20;
		break;
	case DDENS | EIGHTINCH:
		/* There are no real standards here, so use the Cromemco one */
		d->spt = 32;
		break;
	case HDENS:
		/* IBM PC style 5.25" high density. 3.5" is 36/18 spt */
		d->spt = 30;
		break;
	}
	d->ss >>= d->bs;
	if (!d->ds)
		d->ds = d->spt / 2;
	return 0;
}

int gm8x9_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
	flag;
	return gm8x9_transfer(minor, true, rawflag);
}

int gm8x9_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
	flag;
	rawflag;
	minor;
//    return 0;
	return gm8x9_transfer(minor, false, rawflag);
}


/* TODO discard routine to init this lot */