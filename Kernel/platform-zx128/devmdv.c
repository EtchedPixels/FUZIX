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

#define MAX_MDV		2		/* for now */

/* Should probably have a max and a max open to keep the maps managable */
static unsigned char mdvmap[MAX_MDV][256];
static uint8_t mdv_valid;

/* Used by the asm helpers */
uint8_t mdv_sector;
uint8_t *mdv_buf;
uint8_t mdv_hdr_buf[15];
uint16_t mdv_len;

static int mdv_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
	int err;
	irqflags_t irq;

	if (rawflag)
		goto bad;

	mdv_motor_on(minor);
	/* FIXME: support swap ? */
	mdv_sector = mdvmap[minor][udata.u_buf->bf_blk];
	mdv_buf = udata.u_buf->bf_data;

	irq = di();	
	if (is_read)
		err = mdv_bread();
	else
		err = mdv_bwrite();
	irqrestore(irq);
	mdv_motor_off();
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
	t = tmpbuf();
	mdv_buf = t;
	mdv_sector = 1;
	err = mdv_bread();
	if (err) {
		mdv_sector = 128;
		err = mdv_bread();
		if (err) {
			kprintf("mdv_open: maps bad: %d\n", err);
			udata.u_error = ENXIO;
			return -1;
		}
		kprintf("mdv_open: had to use secondary map\n");
	}
	brelse(t);	
	mdv_valid |= 1 << minor;
	return 0;
}

int mdv_close(uint8_t minor)
{
	/* Simple approach for now */
	mdv_valid &= ~(1 << minor);	
	return 0;
}
