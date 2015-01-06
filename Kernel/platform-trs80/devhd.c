/*
 *	WD1010 hard disk driver
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devhd.h>
#include <diskgeom.h>

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

#define RATE_4MS	0x08	/* 4ms step rate for hd (conservative) */

#define HDCTRL_SOFTRESET	0x10
#define HDCTRL_ENABLE		0x08
#define HDCTRL_WAITENABLE	0x04

#define HDSDH_ECC256		0x80

/* Used by the asm helpers */
uint8_t hd_page;

/* Swap is scanned for so not constant */
uint16_t swap_dev;

/* Seek and restore low 4 bits are the step rate, read/write support
   multi-sector mode but not all emulators do .. */

#define MAX_HD	4

struct minipart parts[MAX_HD];

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
	/* Should beeturning READY, and maybe SEEKDONE */
	return hd_status;
}

int hd_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
	blkno_t block;
	staticfast uint16_t dptr;
	uint16_t ct = 0;
	staticfast uint8_t tries;
	uint8_t err = 0;
	uint8_t cmd = HDCMD_READ;
	uint8_t head;
	uint8_t sector;
	uint16_t nblock;
	staticfast uint16_t cyl;
	uint8_t dev = minor >> 4;
	staticfast struct minipart *p;

	p = &parts[dev];

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

	/* TRS80 hard disk are 32 sectors/track, 256 byte sectors */

	hd_precomp = p->g.precomp;
	hd_seccnt = 1;

	sector = block;
	sector = ((sector << 1) & 0x1E) | 0x01;

	cyl = block >> 4;

	/* Do the maths once and on 16 bit numbers */
	head = cyl % p->g.head;
	cyl /= p->g.head;
	if (minor)
		cyl += p->cyl[(minor-1)&0x0F];

	while (ct < nblock) {
		/* Head next bits, plus drive */
		hd_sdh = 0x80 | head | (dev << 3);
		hd_secnum = sector;
		/* cylinder bits */
		hd_cyllo = cyl & 0xFF;
		hd_cylhi = cyl >> 8;

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
				hd_cmd = HDCMD_RESTORE | p->g.seek;
				if (hd_waitready() & 1)
					kprintf("hd%d: restore error %z\n", minor, err);
			}
		}
		/* FIXME: should we try the other half and then bale out ? */
		if (tries == 3)
			goto bad;
		ct++;
		dptr += 256;
		sector++;
		/* Cheaper than divison! */
		if (sector == 33) {
			sector = 1;
			head++;
			if (head == p->g.head) {
				head = 0;
				cyl++;
			}
		}
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
	uint8_t dev = minor >> 4;
	flag;
	if (dev > MAX_HD || parts[dev].g.head == 0 ||
		(minor && parts[dev].cyl[(minor-1)&0x0F] == 0xFFFF)) {
		udata.u_error = ENODEV;
		return -1;
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

/*------------------- below this is thrown after boot ---------------- */
DISCARDABLE

/*
 *	Mini part processing. This and mbr and other things all one day
 *	need to become a bit more unified.
 */
static void hd_swapon(struct minipart *p, unsigned int d, unsigned int i)
{
	uint16_t cyls;
	if (i == 14 || p->cyl[i+1] == 0xFFFF)
		cyls = p->g.cyl;
	else
		cyls = p->cyl[i+1];
	cyls -= p->cyl[i];
	cyls *= p->g.head;
	/* This is now the number of sets of 32 sectors (16K) in swap.
	   We need 32K per process: hardwire it here - FIX if you change
	   the mapping model */

	swap_dev = d << 4 + i + 1 + 0x100;

	if (cyls >= MAX_SWAPS)
		cyls = MAX_SWAPS - 1;
	for (i = 0; i < cyls; i++) {
		swapmap_add(i);
	}
	kputs("swap-");
}

void hd_probe(void)
{
	unsigned int dev = 0;
	unsigned int i;
	uint8_t *d = tmpbuf();
	/* Second half of second block */
	struct minipart *p = (struct minipart *)(d + 128);
	for (dev = 0; dev < 4; dev++) {
		hd_sdh = 0x80 | (dev << 3);
		hd_cmd = HDCMD_RESTORE | RATE_4MS;
		if (hd_waitready() & 1) {
			if ((hd_err & 0x12) == 0x12)
				continue;
		}
		hd_seccnt = 1;
		hd_sdh = 0x80 | (dev << 3);
		hd_secnum = 2;
		hd_cyllo = 0;
		hd_cylhi = 0;
		hd_cmd = HDCMD_READ;
		if (hd_waitdrq() & 1)
			continue;
		if((hd_xfer(1, (uint16_t)d) & 0x41) != 0x40)
			continue;
		kprintf("hd%c: ", dev + 'a');
		if (p->g.magic != MP_SIG_0) {
			p->g.cyl = 1;
			p->g.head = 1;
			p->g.sec = 32;
			p->g.precomp = 0;
			p->g.seek = 10;
			p->g.secsize = 8;
			for (i = 0; i < 15; i++)
				p->cyl[i] = 0xFFFFU;
			kputs("(unlabelled)\n");
		} else {
			for (i = 0; i < 15; i++) {
				if (p->cyl[i] != 0xFFFFU) {
					if (p->type[i] == 0x56) {
						/* Configure swap */
						hd_swapon(p, dev, i);
					}
					kprintf("hd%c%d ", dev + 'a', i + 1);
				}
			}
			kputs("\n");
		}
		if (p->g.seek) {
			p->g.seek /= 2;
			if (p->g.seek == 0)
				p->g.seek = 1;
		}
		/* Set the step rate */
		hd_cmd = HDCMD_SEEK | p->g.seek;
		hd_waitready();
		memcpy(&parts[dev], p, sizeof(parts[dev]));
	}
	brelse((bufptr)d);
}
