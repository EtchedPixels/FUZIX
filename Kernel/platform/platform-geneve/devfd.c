/*
 *	Driver for the floppy side of the HFDC controller
 *
 *	We don't use the subprogram interface because it involves mapping
 *	ROM devices and handling all sorts of uglyness in remapping on
 *	interrupts etc - plus its slow.
 */

#include <kernel.h>
#include <fdc.h>
#include "hfdc.h"

#define CMD_FDREAD		0x5D
#define CMD_FDWRITE		0xA0
#define CMD_FDWRITE_P		0xA8	/* add precomp */


/* Standard precompensation is 125ns, 41ns for 1Mbit 20ns for 2Mbit. The
   controller requires we actually set this up */

/* TODO: should we hand back a different set depending on the switches */
static struct fdcinfo fdcap = {
	0,
	0,
	0,			/* Precomp ?? */
	FDF_DD | FDF_HD| FDF_DS | FDF_SEC256 | FDF_SEC512,
	36,
	80,
	2,
	0
};

static uint8_t mode[MAX_FD];

#define NUM_MODES	11	/* for now */

static struct fdcinfo fdcmodes[NUM_MODES] = {
	/* Lot of formats. */

	/* First block. 40 track single density */

	/* SSSD 256 bytes/sector */
	{
	 0,
	 FDTYPE_GENSD40 FDF_SEC256,
	 9,
	 40,
	 1,
	 0 },

	/* DSSD 256 bytes/sector, double sided */
	{
	 0,
	 FDTYPE_GENSD40D FDF_SEC256,
	 9,
	 40,
	 2,
	 0 },

	/* Double density 16 sectors per track */

	/* SSDD 16 spt 40 track */
	{
	 0,
	 FDTYPE_GENDD4016,
	 FDF_DD | FDF_SEC256,
	 16,
	 40,
	 1,
	 0 },

	/* SSDD 16 spt 80 track */
	{
	 0,
	 FDTYPE_GENDD8016,
	 FDF_DD | FDF_SEC256,
	 16,
	 80,
	 1,
	 0 },
	/* DSDD 16 spt 80 track */
	{
	 0,
	 FDTYPE_GENDD8016D,
	 FDF_DD | FDF_SEC256,
	 16,
	 80,
	 2,
	 0 },

	/* Same again but 18spt */

	/* SSDD 18 spt 40 track */
	{
	 0,
	 FDTYPE_GENDD4018,
	 FDF_DD | FDF_SEC256,
	 18,
	 40,
	 1,
	 0 },
	/* SSDD 18 spt 80 track */
	{
	 0,
	 FDTYPE_GENDD8018,
	 FDF_DD | FDF_SEC256,
	 18,
	 80,
	 1,
	 0 },
	/* DSDD 18 spt 80 track */
	{
	 0,
	 FDTYPE_GENDD8018D,
	 FDF_DD | FDF_SEC256,
	 18,
	 80,
	 2,
	 0 },
	/* DSHD 36 spt 80 track */
	{
	 0,
	 FDTYPE_GENHD80D,
	 FDF_DD | FDF_SEC256,
	 36,
	 80,
	 2,
	 0 },

	/* PC 360K floppy */
	{			/* DSDD */
	 0,
	 FDTYPE_PC360,
	 FDF_DD | FDF_SEC512,
	 9,
	 40,
	 2,
	 0 },
	/* PC 360K floppy */
	{			/* DSDD */
	 0,
	 FDTYPE_PC12,
	 FDF_HD | FDF_SEC512,
	 15,
	 80,
	 2,
	 0 }
};

/* This holds precomputed values for the drive */
struct diskprop {
	unsigned size;
	unsigned sectors;
	unsigned heads;
	unsigned shift;
	unsigned features;
	unsigned precomp;
	uint8_t mode;
	uint8_t ddel;
};

static struct diskprop diskprop[MAX_FD];

/* Select the drive to work on. Also set up the clocking to be 500KHz
   for 3/3.5/5.25" SD/DD, anmd 1MHz for 5.25" HD or 8" */
static int fd_select(unsigned int minor, unsigned int hd)
{
	unsigned int cmd = 0x28 | minor;
	if (hd == 0)
		cmd |= 0x20;
	return hfdc_execute_simple(cmd, 1);
}

static int fd_transfer(uint_fast8_t minor, bool is_read,
		       uint_fast8_t rawflag)
{
	struct diskprop *dp = diskprop + minor;
	uint8_t *p;
	unsigned cyl;
	unsigned retry = 0;

	if (rawflag == 2)	/* No floppy swappy */
		goto bad;

	hfdc_sync();

	if (fd_select(minor, (dp->featurs & FDC_HD)))
		goto bad;

	if (rawflag) {
		if (d_blkoff(9 - dp->shift))
			return -1;
	} else {
		udata.u_nblock <<= dp->shift;
		udata.u_block <<= dp->shift;
	}

	/* TODO: double step */
	while (ct < udata.u_nblock) {
		hfdc_sync();
		if (!is_read)
			hfdc_copy_to_device(udata.u_dptr, dp->size,
					    rawflag);
		p = hfdc_block;
		/* For now always use the RAM start */
		*p++ = 0;
		*p++ = 0;
		*p++ = 0;
		*p = udata.u_block & dp->sectors;
		if (!(dp->features & FDC_SEC0))
			(*p)++;
		p++;
		cyl = udata.u_block / dp->sectors;

		if (dp->heads = 2) {
			*p++ = (cyl & 1) | dp->headr;	/* sector size dependent */
			cyl >>= 1;
		} else
			*p++ = dp->headr;
		/* We do a sector at a time. We should be smart and do blocks
		   providing it doesn' t cross a boundary and we have enough RAM */
		*p++ = 1;
		/* FIXME: do we do retries on write on floppy ourselves or
		   via controller ?? */
		*p++ = 0;	/* Max retries */
		*p++ = dp->mode;	/* CRC on, step as per drive, SD/DD */
		*p++ = 0xB7;	/* Termination causes */
		*p = dp->ddel;	/* Sets the sec size */

		if (is_read)
			cmd = CMD_FDREAD;
		else if (cyl >= dp->precomp) {
			/* 100ns. */
			cmd = CMD_FDWRITE_P | 1;
		else
			cmd = CMD_FDWRITE;

		if (hfdc_execute(cmd, 1)) {
			hfdc_error();
			/* We own second level retry - restore to align heads */
			if (retry++ < 3) {
				hfdc_recover(minor);
				goto retry;
			}
bad:
			hfdc_execute_simple(0x01, 1);
			udata.u_error = EIO;
			return -1;
		} else if (is_read)
				hfdc_copy_from_device(udata.u_dptr,
						      dp->size, rawflag);
		udata.u_block++;
		udata.u_dptr += dp->size;
	}
	/* Deselect drives */
	hfdc_execute_simple(0x01, 1);
	return udata.u_nblock << (7 + dp->shift);
}

static void fd_setup(uint_fast8_t minor, uint_fast8_t step)
{
	/* Copy the features for the mode into the device parameters */
	struct fdcinfo *f = fdcmodes + mode[minor];
	struct diskprop *dp = diskprop + minor;
	uint8_t s;

	dp->features = f->features;
	dp->size = f->features & FDF_SECSIZE;
	switch (dp->size) {
	case 128:
		s = 0;
		break;
	case 256:
	default:
		s = 1;
		break;
	case 512:
		s = 2;
		break;
	}
	dp->shift = s;
	dp->sectors = f->sectors;
	dp->heads = f->heads;
	if (dp->features & (FDF_DD|FDF_HD)) {
		if (f->tracks <= 40)
			dp->precomp = 21;	/* DD 35/40 track drives */
		else
			dp->precomp = 42;	/* Check */
	} else
		dp->precomp = 255;	/* None */
	/* Now calculate the mode and ddel loads */
	dp->ddel = s << 4;
	if (step == 255)
		dp->mode &= 7;
	else
		dp->mode = step;
	if (!(dp->features & (FDF_DD|FDFHD)))
		dp->mode |= 0x10;	/* FM */
}

int fd_open(uint_fast8_t minor, uint16_t flag)
{
	flag;
	if (minor >= MAX_FD) {
		udata.u_error = ENODEV;
		return -1;
	}
	if (!fd_present(minor) && !(flag & O_NDELAY)) {
		udata.u_error = EIO;
		return -1;
	}
	fd_setup(minor, 255);
	return 0;
}

/*
 *	Read and write wrap fd_transfer
 */
int fd_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
	flag;
	return fd_transfer(minor, true, rawflag);
}

int fd_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
	flag;
	rawflag;
	minor;
	return fd_transfer(minor, false, rawflag);
}

/*
 *	Floppy disk ioctls
 */
int fd_ioctl(uint_fast8_t minor, uarg_t request, char *buffer)
{
	uint8_t s;
	struct fdcstep step;

	switch (request) {
	case FDIO_GETCAP:
		fdcap.mode = mode[minor];
		return uput(&fdcap, buffer, sizeof(struct fdcinfo));
	case FDIO_GETMODE:
		s = ugetc(buffer);
		if (s >= NUM_MODES) {
			udata.u_error = EINVAL;
			return -1;
		}
		return uput(fdcmodes + s, buffer, sizeof(struct fdcinfo));
	case FDIO_SETMODE:
		s = ugetc(buffer);
		if (s >= NUM_MODES) {
			udata.u_error = EINVAL;
			return -1;
		}
		mode[minor] = s;
		fd_setup(minor, 255);
		return 0;
	case FDIO_SETSTEP:
		if (uget(buffer, &step, sizeof(step)))
			return -1;
		step.steprate;
		/* Convert  step rate ms */
		s = 0;
		while (step.steprate >= 1)
			step.steprate >>= 1;
		s++;
		fd_setup(minor, s);
		return 0;
	case FDIO_FMTTRK:
		/* TODO */
		return -1;
	case FDIO_RESTORE:
		fd_selected = 255;	/* Force a re-configure */
		return do_fd_restore(minor);
	}

	return -1;
}


static void fd_set_bits(unsigned drive, unsigned bits)
{
	bits &= 3;
	switch (bits) {
	case 0:		/* SS, 40 track, 16ms seek */
		mode[drive] = 0;
		fd_setup(drive, 4);
		break;
	case 1:		/* SS, 40 track, 8m seek */
		mode[drive] = 0;
		fd_setup(drive, 3);
		break;
	case 2:		/* 2m seek 40 or 80 track */
		mode[drive] = 7;
		fd_setup(drive, 2);
		break;
	case 3:		/* 1.44MB floppy */
		mode[drive] = 9;
		fd_setup(drive, 3);
		break;
	}
}

/*
 *	At boot set the drives up from the dip switches
 */
void hfdc_init(void)
{
	unsigned n = hfdc_read_switches();
	fd_set_bits(0, n >> 2);
	fd_set_bits(1, n);
	fd_set_bits(2, n >> 6);
	fd_set_bits(3, n >> 4);
}
