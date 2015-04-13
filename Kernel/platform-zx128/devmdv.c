/*
 *	Sinclair Interface One + Microdrives, mapped as if they were a
 *	floppy disk.
 *
 *	First draft: motor control not yet made smart
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devmdv.h>

#define MAX_MDV		4		/* for now */

/* Should probably have a max and a max open to keep the maps managable */
static unsigned char mdvmap[MAX_MDV][256];
static uint8_t mdv_valid;

/* Used by the asm helpers */
uint8_t *mdv_buf;
uint8_t mdv_hdr_buf[15];
uint8_t mdv_w_hdr_buf[15+12];

uint8_t mdv_sector;
uint16_t mdv_len;
uint8_t mdv_page;
uint8_t mdv_csum;
static uint8_t mdv_tick;
static uint8_t mdv_minor;

void mdv_timer(void)
{
	if (mdv_tick) {
		mdv_tick--;
		if (mdv_tick == 0)
			mdv_motor_off();
	}
}

static int mdv_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
	int err;
	irqflags_t irq;
	uint16_t block, nblock;
	uint8_t on = 0;
	uint8_t altbank = 0;	/* Using alternate banks ? */

	if (rawflag == 0) {
		mdv_buf = udata.u_buf->bf_data;
		block = udata.u_buf->bf_blk;
		nblock = 1;
		mdv_page = 0;
	} else if (rawflag == 1) {
		/* Direct to user */
		if (((uint16_t)udata.u_offset|udata.u_count) & BLKMASK)
			goto bad;
		mdv_buf = (uint8_t *)udata.u_base;
		nblock = udata.u_count >> 9;
		block = udata.u_offset >> 9;
		mdv_page = udata.u_page;
	} else {
		/* Microdrive swap awesomeness */
		nblock = swapcnt >> 9;
		block = swapblk;
		mdv_page = swapproc->p_page;
		mdv_buf = swapbase;

		/* Platform specific magic time. We know the swapper
		   will do I/O to/from "us" (easy) or from the
		   other process, in which case we need to juggle
		   banks half way */
		if (swapproc != udata.u_ptab) {
			altbank = 1;
			mdv_buf = (uint8_t *)0xC000;
			mdv_page = 6;	/* Switched bank */
		}
	}

	irq = di();
	if (mdv_minor != minor) {
		mdv_tick = 0;
		mdv_motor_off();
	}
	if (mdv_tick == 0)
		on = 1;
	mdv_tick = 250;
	irqrestore(irq);
	if (on)
		mdv_motor_on(minor + 1);

	while(nblock--) {
		mdv_sector = mdvmap[minor][block++];
		kprintf("Load sector %d to %d:%x\n", mdv_sector, mdv_page, mdv_buf);
		irq = di();
		if (is_read)
			err = mdv_bread();
		else
			err = mdv_bwrite();
		irqrestore(irq);
		if (err) {
			kprintf("mdv%d: %c error %d\n", minor, "wr"[is_read],
				err);
			goto bad;
		}
		mdv_buf += 512;
		/* Switch bank */
		if (altbank && mdv_buf == 0x0000) {	/* Wrapped */
			mdv_page = swapproc->p_page;
			mdv_buf = (uint8_t *)0xC000;
		}
	}
	return 0;
bad:
	udata.u_error = EIO;
	return -1;
}

int mdv_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
	flag;
	return mdv_transfer(minor, true, rawflag);
}

int mdv_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
	flag;
	return mdv_transfer(minor, false, rawflag);
}

int mdv_open(uint8_t minor, uint16_t flag)
{
	uint8_t *t;
	int err;

	flag;

	if (minor >= MAX_MDV) {
		udata.u_error = ENODEV;
		return -1;
	}
	mdv_motor_on(minor + 1);
	t = tmpbuf();
	mdv_buf = t;
	mdv_sector = 1;
	mdv_page = 0;
	err = mdv_bread();
	if (err) {
		mdv_sector = 128;
		err = mdv_bread();
		if (err) {
			kprintf("mdv_open: maps bad: %d\n", err);
			mdv_motor_off();
			udata.u_error = ENXIO;
			return -1;
		}
		kprintf("mdv_open: had to use secondary map\n");
	}
	memcpy(mdvmap[minor], t, 256);
	brelse(t);	
	mdv_valid |= 1 << minor;
	mdv_motor_off();
	return 0;
}

int mdv_close(uint8_t minor)
{
	/* Simple approach for now */
	mdv_valid &= ~(1 << minor);	
	return 0;
}
