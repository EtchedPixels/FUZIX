/*
 *	WD1010 hard disk driver
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devhd.h>

__sfr __at 0xC0 hd_wpbits;
__sfr __at 0xC1 hd_ctrl;
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
#define HDCMD_VERIFY	0x40
#define HDCMD_FORMAT	0x50
#define HDCMD_INIT	0x60
#define HDCMD_SEEK	0x70

/* Seek and restore low 4 bits are the step rate, read/write support
   multi-sector mode but not all emulators do .. */

#define MAX_HD	4


static uint8_t hd_waitready(void)
{
	uint8_t st;
	do {
		st = hd_status;
	} while (st & 0x80);
	return st;
}

/* FIXME: move this to asm in _COMMONMEM and support banks and swap */
static uint8_t hd_xfer(bool is_read, uint16_t addr)
{
	int ct = 256;
	uint8_t *ptr = (uint8_t *)addr;

	if (!(hd_status & 0x08))
		return hd_status;
	if (is_read) {
		/* Ought to check DRQ per byte ? */
		while (ct--) {
			*ptr++ = hd_data;
		}
		return 0;
	} else {
		while (ct--) {
			hd_data = *ptr++;
		}
		return 0;
	}
}

/*
 *	We only support normal block I/O for the moment. We do need to
 *	add swapping!
 */

static int hd_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
	blkno_t block;
	uint16_t dptr;
	int ct = 0;
	int tries;
	uint8_t err = 0;
	uint8_t cmd = HDCMD_READ;
	uint8_t head;
	uint8_t sector;

	if (rawflag)
		goto bad2;

	if (!is_read)
		cmd = HDCMD_WRITE;

	dptr = (uint16_t) udata.u_buf->bf_data;
	block = udata.u_buf->bf_blk;

	/* We assume 32 sectors per track for now. From our 512 byte
	   PoV that's 16 */

	/* For our test purposes we use a disk with 32 sectors, 8 heads so
	   our blocks map out as   ccccccccCCCCCCCCHHHSSSS */
	hd_seccnt = 1;
	sector = ((block & 15) << 1) + 1;
	hd_secnum = sector;
	hd_cyllo = (block >> 7) & 0xFF;
	hd_cylhi = (block >> 15) & 0xFF;
	head = (block >> 4) & 7;

	hd_precomp = 0;		/* FIXME */
	hd_sdh = 0x80 | head | (minor << 3);

	while (ct < 2) {
		for (tries = 0; tries < 4; tries++) {
			hd_cmd = cmd;
			err = hd_waitready();
			if (err == 0) {
				err = hd_xfer(is_read, dptr);
				break;
			}
			if (tries > 1) {
				hd_cmd = HDCMD_RESTORE;
				hd_waitready();
			}
		}
		/* FIXME: should we try the other half and then bale out ? */
		if (tries == 3)
			goto bad;
		ct++;
		dptr += 256;
		hd_secnum = sector + 1;
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
	if (minor > MAX_HD) {
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
