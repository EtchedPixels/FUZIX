#/*
 *	80bus floppy controllers
 *
 *	Henelec/GM805
 *		Not this driver
 *	Nascom	WD1793
 *		SD/DD, DD/HD select by hardware
 *	GM809	WD1797
 *		SD/DD, DD/HD select by hardware
 *	GM829	WD1797
 *		SD/DD, DD/HD switch, no 5.25" HD speed control, SASI option
 *	GM849/A	WD2793
 *		SD/DD, DD/HD switch, 5.25" HD speed control, 8 drives, SASI
 *	MAP80
 *		As GM809 ?
 */

#include <kernel.h>
#include <kdata.h>
#include <fdc.h>
#include <fdc80.h>

#define MAX_FD		8
#define	MAX_SKEW	32

struct fdc {
	uint8_t track;		/* Saved track value */
	uint8_t spt;		/* Sectors per track (including both sides if DS) */
	uint8_t bs;		/* Block shift to sectors */
	uint16_t ss;		/* Sector size */
	uint8_t ds;		/* Sector that starts second side, 255 = SS */
	uint8_t skewtab[MAX_SKEW];	/* Skew table TODO */
	uint8_t steprate;
	uint8_t select;		/* Byte to write to select/config */
};

static struct fdc fdcinfo[MAX_FD];
static struct fdc *last_dev;
static uint8_t fdctype;
static uint8_t fdcstatus;

static uint16_t rdcmd;
static uint16_t wrcmd;

static const uint8_t dentab[3][5] = {
	/* Nascom: SD DD HD 8SD 8DD */
	{ 0x20, 0x60, 0xFF, 0x20, 0x60 },
	/* GM809/GM829/MAP80 */
	{ 0x10, 0x00, 0x20, 0x10, 0x20 },
	/* GM849 */
	{ 0x10, 0x00, 0xA0, 0x50, 0x40 }
};

/* Side bits by card - 0 for select, 1 is onto command */
static const uint8_t side1[3][2] = {
	{ 0x10, 0x00 },
	{ 0x00, 0x02 },
	{ 0x08, 0x00 }
};

/* Drive select and set up by card type, including motor poke */
static int fdc80_config(unsigned dev, unsigned density)
{
	uint8_t r = dentab[fdctype][density];

	if (r == 0xFF)
		return -1;

	if (fdctype == FDC_GM849)
		r |= dev;
	else
		r |= 1 << dev;
	return r;
}

static unsigned fdc80_select(register struct fdc *fdc, unsigned side)
{
	uint_fast8_t r;

	r = fdc->select;
	if (side)
		r |= side1[fdctype][0];
	out(0xE4, r);

	if (fdc == last_dev)
		return 0;

	out(0xE1, fdc->track);
	last_dev = fdc;

	return 1;
}

static unsigned fdc80_do_seek(register struct fdc *fdc, unsigned track)
{
	uint8_t status;
	fdc80_track = track;
	fdc->track = -1;
	status = fdc80_seek((fdcstatus << 8) | 0x14 | fdc->steprate);
	if (status & 0x10) {
		kprintf("fd: seek error %2x\n", status);
		return status;
	}
	fdc->track = track;
	return 0;
}

static unsigned fdc80_do_restore(register struct fdc *fdc)
{
	uint8_t status = fdc80_cmd((fdcstatus << 8) | 0x00 | fdc->steprate);
	fdc->track = -1;
	if (status & 0x10)
		return status;
	fdc->track = 0;
	return 0;
}

static unsigned fdc80_test_restore(register struct fdc *fdc)
{
	uint8_t status = fdc80_cmd((fdcstatus << 8) | 0x04 | fdc->steprate);
	fdc->track = -1;
	if (status & 0x10)
		return status;
	fdc->track = 0;
	return 0;
}

static int fdc80_transfer(uint_fast8_t minor, bool is_read, uint_fast8_t rawflag)
{
	register struct fdc *fdc = fdcinfo + minor;
	register uint16_t track;
	register uint16_t sector;
	uint16_t ct = 0;
	unsigned tries;
	irqflags_t irqflags;
	uint8_t sv;
	uint8_t side;
	uint8_t err;

	/* No floppy swap */
	if (rawflag == 2)
		goto bad2;
	if (rawflag) {
		fdc80_iopage = udata.u_page;
		if (d_blkoff(9 - fdc->bs))
			return -1;
	} else {
		fdc80_iopage = 0;
		udata.u_nblock <<= fdc->bs;
		udata.u_block <<= fdc->bs;
	}
	/* Now loop through each sector */
	while (ct < udata.u_nblock) {
		side = 0;
		sv = 0;
		/* TODO: skew fdc->skew[] */
		sector = udata.u_block % fdc->spt;
		track = udata.u_block / fdc->spt;
		if (sector >= fdc->ds) {
			sector -= fdc->ds;
			side = 1;
			sv = side1[fdctype][1];
		}
		fdc80_select(fdc, side);
		/* Make multiple attempts to get the data. If it keeps failing try
		   restoring the head and seeking in order to re-align */
		for (tries = 0; tries < 5; tries++) {
			/* Try to get the requested track */
			if (in(0xE1) != track) {
				if ((err = fdc80_do_seek(fdc, track))) {
					fdc80_do_restore(fdc);
					continue;
				}
			}
			/* Set up the sector register, hardcode base 1 for now */
			out(0xE2, sector + 1);
			fdc80_dptr = (uint16_t) udata.u_dptr;
			/* The timing on these is too tight to do with interrupts on */
			plt_disable_nmi();
			irqflags = di();
			if (is_read)
				err = fdc80_readsec(rdcmd | sv);
			else
				err = fdc80_writesec(wrcmd | sv);
			irqrestore(irqflags);
			plt_enable_nmi();
			/* It worked - exit then inner retry loop and move on */
			if (err == 0)
				break;
			/* Force a head seek */
			if (tries > 1)
				fdc80_do_restore(fdc);
		}
		if (tries == 5)
			goto bad;
		udata.u_block++;
		udata.u_dptr += fdc->ss;
		ct++;
	}
	/* And done */
	return udata.u_nblock << (9 - fdc->bs);
bad:
	kprintf("fd%d: error %x\n", minor, err);
bad2:
	udata.u_error = EIO;
	return -1;
}

int fdc80_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
	return fdc80_transfer(minor, true, rawflag);
}

int fdc80_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
	return fdc80_transfer(minor, false, rawflag);
}

/* Will need proper info passing in */
static void fdc80_setup(struct fdc *fdc, unsigned density)
{
	/* Assume 3.5" when HD for the moment */
	fdc->select = fdc80_config(fdc - fdcinfo, density);
	fdc->bs = 0;
	fdc->ss = 512;
	fdc->steprate = 1;	/* Assume 6ms */
	switch (density) {
	case 2:		/* PC HD 1440K */
		fdc->spt = 36;
		fdc->ds = 18;
		break;
	case 1:		/* PC DD 360K */
		fdc->spt = 18;
		fdc->ds = 9;
		break;
	case 0:		/* SD media - god knows */
		fdc->spt = 36;
		fdc->bs = 2;
		fdc->ss = 128;
		fdc->ds = 255;
		break;
	}
}

/*
 *	Attempt to autodetect the media
 */

static int fdc80_media(unsigned minor, struct fdc *fdc)
{
	uint8_t status;
	uint8_t s = fdc80_config(minor, 2);
	/* Try HD */
	if (s != 0xFF) {
		out(0xE4, s);
		if (fdc80_test_restore(fdc) == 0)
			return 2;
	}
	/* Try DD */
	s = fdc80_config(minor, 1);
	out(0xE4, s);
	if (fdc80_test_restore(fdc) == 0)
		return 1;
	/* Try SD */
	s = fdc80_config(minor, 0);
	out(0xE4, s);
	if (fdc80_test_restore(fdc) == 0)
		return 0;
	/* TODO: 8" handlers */
	return -1;
}

/*
 *	Open a floppy device
 */
int fdc80_open(uint_fast8_t minor, uint16_t flags)
{
	register struct fdc *fdc = fdcinfo + minor;
	int density;
	if (fdctype == FDC_NONE || minor >= MAX_FD ||
			(fdctype != FDC_GM849 && minor > 3)) {
		udata.u_error = ENODEV;
		return -1;
	}
	/* Try and detect */
	density = fdc80_media(minor, fdc);
	if (density < 0 && !(flags & O_NDELAY)) { 
		udata.u_error = EIO;
		return -1;
	}
	/* Fudged for now */
	fdc80_setup(fdc, density);
	return 0;
}

/* To discard section ? */

unsigned fdc80_probe(void)
{
	/* See if we have a working FDC track register */
	out(0xE2, 1);
	if (in(0xE2) != 1)
		return 0;
	out(0xE2, 2);
	if (in(0xE2) != 2)
		return 0;
	/* We probably have an FDC, now play guess which */
	out(0xE4, 2);
	if ((in(0xE4) & 0x0F) == 2) {
		out(0xE4, 4);
		if ((in(0xE4) & 0x0F) == 4) {
			/* Looks like a Nascom FDC */
			fdctype = FDC_NASCOM;
			fdcstatus = 0xE5;
			rdcmd = 0xE588;
			wrcmd = 0xE5A8;
			kputs("Nascom FDC at 0xE0\n");
			return FDC_NASCOM;
		}
	}
	/* E4 does not mirror drive selection. Therefore we are a Gemini or MAP80 */
	if (in(0xE5) & 0x80) {
		fdctype = FDC_GM849;
		fdcstatus = 0xE4;
		rdcmd = 0xE488;
		wrcmd = 0xE4A8;
		/* Gemini GM849 or GM849A */
		kputs("Gemini GM849 FDC at 0xE0\n");
		return FDC_GM849;
	}
	/* Gemini 809 or 829 (we don't distinguish), or MAP80 VFDC */
	fdctype = FDC_GM809;
	fdcstatus = 0xE4;
	rdcmd = 0xE480;
	wrcmd = 0xE4A0;
	kputs("Gemini GM809/29 or compatible at 0xE0\n");
	return FDC_GM809;
}
