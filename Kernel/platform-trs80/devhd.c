/*
 *	WD1010 hard disk driver
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devhd.h>

__sfr __at 0xC0 hd_wpbits;	/* Write protect and IRQ (not used) */
__sfr __at 0xC1 hd_ctrl;	/* Reset and enable bits */
__sfr __at 0xC8 hd_data;
__sfr __at 0xC9 hd_precomp;	/* W/O */
__sfr __at 0xC9 hd_err;		/* R/O */
__sfr __at 0xCA hd_seccnt;
__sfr __at 0xCB hd_secnum;
__sfr __at 0xCC hd_cyllo;
__sfr __at 0xCD hd_cylhi;
__sfr __at 0xCE hd_sdh;
__sfr __at 0xCF hd_status;	/* R/O */
__sfr __at 0xCF hd_cmd;

#define HDCMD_RESTORE	0x10
#define HDCMD_READ	0x20
#define HDCMD_WRITE	0x30
#define HDCMD_VERIFY	0x40	/* Not on the 1010 later only */
#define HDCMD_FORMAT	0x50
#define HDCMD_INIT	0x60	/* Ditto */
#define HDCMD_SEEK	0x70

#define RATE_2MS	0x04	/* 2ms step rate for hd (conservative) */

#define HDCTRL_SOFTRESET	0x10
#define HDCTRL_ENABLE		0x08
#define HDCTRL_WAITENABLE	0x04

#define HDSDH_ECC256		0x80

/* Used by the asm helpers */
uint8_t hd_page;

/* Seek and restore low 4 bits are the step rate, read/write support
   multi-sector mode but not all emulators do .. */

#define MAX_HD	4

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

static int hd_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
	blkno_t block;
	uint16_t dptr;
	uint16_t ct = 0;
	int tries;
	uint8_t err = 0;
	uint8_t cmd = HDCMD_READ;
	uint8_t head;
	uint8_t sector;
	uint16_t nblock;

	if (rawflag == 0) {
		dptr = (uint16_t)udata.u_buf->bf_data;
		block = udata.u_buf->bf_blk;
		nblock = 2;
		hd_page = 0;		/* Kernel */
	} else if (rawflag == 2) {
		nblock = swapcnt >> 8;	/* in 256 byte chunks */
		dptr = (uint16_t)swapbase;
		hd_page = swapproc->p_page;
		block = swapblk;
	} else
		goto bad2;

	if (!is_read)
		cmd = HDCMD_WRITE;


	/* We assume 32 sectors per track for now. From our 512 byte
	   PoV that's 16 */

	/* For our test purposes we use a disk with 32 sectors, 4 heads so
	   our blocks map out as   00cccccc ccCCCCCC CCHHSSSS

	   This matches a real ST506 which is 153 cyls, 4 heads, 32 sector */

	hd_precomp = 0x20;	/* For now, matches an ST506 */
	hd_seccnt = 1;
	block <<= 1;		/* Into 256 byte blocks, limits us to 32MB
				   so ought to FIXME */

	while (ct < nblock) {
		/* 32 sectors per track assumed for now */
		sector = (block & 31) + 1;
		head = (block >> 5) & 3;
		/* Head next bits, plus drive */
		hd_sdh = 0x80 | head | (minor << 3);
		hd_secnum = sector;
		/* cylinder bits */
		hd_cyllo = (block >> 7) & 0xFF;
		hd_cylhi = (block >> 15) & 0xFF;

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
		/* FIXME: should we try the other half and then bale out ? */
		if (tries == 3)
			goto bad;
		ct++;
		dptr += 256;
		block ++;
	}
	return 1;
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
	if (minor >= MAX_HD) {
		udata.u_error = ENODEV;
		return -1;
	}
	hd_sdh = 0xA0 | (minor << 3);
	hd_cmd = HDCMD_RESTORE | RATE_2MS;
	if (hd_waitready() & 1) {
		if ((hd_err & 0x12) == 0x12)
			return -ENODEV;
	}
	return 0;
}

int hd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
	flag;
	return hd_transfer(minor, true, rawflag);
}

int hd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
	flag;
	return hd_transfer(minor, false, rawflag);
}
