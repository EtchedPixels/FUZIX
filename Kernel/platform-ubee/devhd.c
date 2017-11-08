/*
 *	The MicroBee 128 shipped with a WD1002-05 and various small drives.
 *
 *	The driver is based on the TRS80 WD1010 driver. The 1002-05 is a 1010
 *	with additional glue chips that also manage and front up a floppy
 *	controller for us including the DMA management. It's actually a better
 *	floppy controller than almost anything that followed it !
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devhd.h>

__sfr __at 0x40 hd_data;
__sfr __at 0x41 hd_precomp;	/* W/O */
__sfr __at 0x41 hd_err;		/* R/O */
__sfr __at 0x42 hd_seccnt;
__sfr __at 0x43 hd_secnum;
__sfr __at 0x44 hd_cyllo;
__sfr __at 0x45 hd_cylhi;
__sfr __at 0x46 hd_sdh;
__sfr __at 0x47 hd_status;	/* R/O */
__sfr __at 0x47 hd_cmd;
__sfr __at 0x48 hd_fdcside;	/* Side select for FDC */
__sfr __at 0x58 fdc_devsel;	/* Whch controller to use */

#define HDCMD_RESTORE	0x10
#define HDCMD_READ	0x20
#define HDCMD_WRITE	0x30
#define HDCMD_VERIFY	0x40	/* Not on the 1010 later only */
#define HDCMD_FORMAT	0x50
#define HDCMD_INIT	0x60	/* Ditto */
#define HDCMD_SEEK	0x70

#define RATE_2MS	0x04	/* 2ms step rate for hd (conservative) */
#define RATE_6MS	0x06	/* 6ms step for floppy (3ms is probably fine
				   for most) */

#define HDSDH_ECC256		0x80

/* Used by the asm helpers */
uint8_t hd_page;

/* Seek and restore low 4 bits are the step rate which we need to sort out
   FIXME */

#define MAX_HD	3	/* devsel 3 means floppy */
#define MAX_FDC	4

/* Standard formats for Microbee:
	40 x 2 x 10 x 512 double sided 5.25
	80 x 2 x 10 x 512 single sided 3.5
	80 x 2 x 10 x 512 double sided 3.5

	For now just use double sided : FIXME */
static int spt[7] = { 16, 16, 16, 10, 10, 10, 10 };
static int heads[7] = { 4, 4, 4, 2, 2, 2, 2 };

/* Wait for the drive to show ready */
static uint8_t hd_waitready(void)
{
	uint8_t st;
	do {
		st = hd_status;
	} while (!(st & 0x40));
	return st;
}

/* Wait for DRQ or an error */
static uint8_t hd_waitdrq(void)
{
	uint8_t st;
	do {
		st = hd_status;
	} while (!(st & 0x09));
	return st;
}

static uint8_t hd_xfer(bool is_read, uint16_t addr)
{
	/* Error ? */
	if (hd_status & 0x01)
		return hd_status;
	if (is_read)
		hd_xfer_in(addr);
	else
		hd_xfer_out(addr);
	/* Should be returning READY, and maybe SEEKDONE */
	return hd_status;
}

/*
 *	We only support normal block I/O for the moment. We do need to
 *	add swapping!
 */

static int hd_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
	uint16_t dptr;
	uint16_t ct = 0;
	int tries;
	uint8_t err = 0;
	uint8_t cmd = HDCMD_READ;
	uint8_t head;
	uint8_t sector;

	if (rawflag) {
		if (rawflag == 1) {
			if (d_blkoff(9))
				return -1;
			/* TODO */
			hd_page = 0xFF;
		} else {
			hd_page = swappage;
		}
	}

	dptr = (uint16_t)udata.u_dptr;

	if (!is_read)
		cmd = HDCMD_WRITE;

	/* We don't touch precomp and hope the firmware set it right */
	hd_seccnt = 1;

	while (ct < udata.u_nblock) {
		uint16_t b = udata.u_block / spt[minor];
		sector = udata.u_block % spt[minor];
		head = b % heads[minor];
		if (minor < MAX_HD) {
			/* ECC, 512 bytes, head and drive */
			hd_sdh = 0xA0 | head | (minor << 3);
		} else {
			/* Floppy setup */
			hd_fdcside = head;
			/* CRC, 512 bytes, head (0/1), FDC unit, fdc number */
			hd_sdh = 0x38 | head | ((minor - MAX_HD) << 1);
		}
		hd_secnum = sector;

		/* cylinder bits */
		b /= heads[minor];
		hd_cyllo = (b >> 7) & 0xFF;
		hd_cylhi = (b >> 15) & 0xFF;

		for (tries = 0; tries < 4; tries++) {
			/* issue the command */
			hd_cmd = cmd;
			/* DRQ will go high once the controller is ready
			   for us */
			err = hd_waitdrq();
			if (!(err & 1)) {
				err = hd_xfer(is_read, dptr);
				/* Ready, no error ? */
				if ((err & 0x41) == 0x40)
					break;
			} else
				kprintf("hd%d: err %x\n", minor, err);

			if (tries > 1) {
				hd_cmd = HDCMD_RESTORE;
				if (hd_waitready() & 1)
					kprintf("hd%d: restore error %z\n", minor, err);
			}
		}
		if (tries == 3)
			goto bad;
		ct++;
		dptr += 512;
		udata.u_block ++;
	}
	return ct << BLKSHIFT;
bad:
	if (err & 1)
		kprintf("hd%d: error %x\n", minor, hd_err);
	else
		kprintf("hd%d: status %x\n", minor, err);
bad2:
	udata.u_error = EIO;
	return -1;
}

int hd_open(uint8_t minor, uint16_t flag)
{
	flag;
	if (minor >= MAX_HD + MAX_FDC) {
		udata.u_error = ENODEV;
		return -1;
	}
	fdc_devsel = 1;
	if (minor <= MAX_HD) {
		hd_sdh = 0xA0 | (minor << 3);
		hd_cmd = HDCMD_RESTORE | RATE_2MS;
	} else {
		hd_sdh = 0x38 | minor << 1;
		hd_cmd = HDCMD_RESTORE | RATE_6MS;
	}
	if (hd_waitready() & 1) {
		if ((hd_err & 0x12) == 0x12)
			return -ENODEV;
	}
	return 0;
}

int hd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
	flag;
	fdc_devsel = 1;
	return hd_transfer(minor, true, rawflag);
}

int hd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
	flag;
	fdc_devsel = 1;
	return hd_transfer(minor, false, rawflag);
}
