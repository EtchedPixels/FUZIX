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
static uint8_t mdv_count[MAX_MDV];

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

	mdv_page = 0;

	if (rawflag == 1) {
		if (d_blkoff(BLKSHIFT))
			return -1;
		/* Direct to user */
		if (((uint16_t)udata.u_offset|udata.u_count) & BLKMASK)
			goto bad;
		mdv_page = udata.u_page;
	} else if (rawflag == 2) {
#ifdef SWAPDEV
		/* Microdrive swap awesomeness */
		mdv_page = swappage;
#else
		/* Attempt to swap when swapping is disabled */
		goto bad;
#endif
	}

	mdv_buf = udata.u_dptr;
	block = udata.u_block;
	nblock = udata.u_nblock;

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
//		kprintf("%d %d:%x\n", mdv_sector, mdv_page, mdv_buf);
		irq = di();
		/* Shouldn't happen but in case */
		if (mdv_tick == 0)
			mdv_motor_on(minor + 1);
		mdv_tick = 250;
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
	if (!mdv_count[minor]) {
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
		tmpfree(t);
		mdv_motor_off();
	}
	mdv_count[minor]++;
	return 0;
}

int mdv_close(uint8_t minor)
{
	/* Simple approach for now */
	mdv_count[minor]--;
	return 0;
}
