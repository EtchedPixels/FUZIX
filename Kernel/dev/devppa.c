#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <blkdev.h>
#include <devppa.h>

/* The PPA is really a full on SCSI to parallel port adapter but we treat it
   all as a magic block device for now.
   
   TODO: write the asm helpers for block mode */

#ifdef CONFIG_BLK_PPA

static void ppa_d_pulse(uint8_t v)
{
	ppa_write_data(v);
	ppa_write_ctrl(0x0C);
	ppa_write_ctrl(0x0E);
	ppa_write_ctrl(0x04);
	ppa_write_ctrl(0x0C);
}

static void ppa_disconnect(void)
{
	ppa_d_pulse(0);
	ppa_d_pulse(0x3C);
	ppa_d_pulse(0x20);
	ppa_d_pulse(0x0F);
}

static void ppa_c_pulse(uint8_t v)
{
	ppa_write_data(v);
	ppa_write_ctrl(0x04);
	ppa_write_ctrl(0x06);
	ppa_write_ctrl(0x04);
	ppa_write_ctrl(0x0C);
}

static static void ppa_connect(void)
{
	ppa_c_pulse(0);
	ppa_c_pulse(0x3C);
	ppa_c_pulse(0x20);
	ppa_c_pulse(0x8F);
}

static uint8_t ppa_select(uint8_t self, uint8_t target)
{
	unsigned timeout = 0;
	uint8_t r;
	ppa_write_data(1 << target);
	ppa_write_ctrl(0x0E);
	ppa_write_ctrl(0x0C);
	ppa_write_data(1 << self);
	ppa_write_ctrl(0x08);
	while(((r = ppa_read_status()) & 0xF0) == 0) {
		timeout++;
		if (timeout > 50000)
			return 0;
	}
	return r & 0xF0;
}

static uint8_t ppa_wait_done(void)
{
	unsigned timeout = 0;
	uint8_t r;
	while(((r = ppa_read_status()) & 0x80) == 0) {
		timeout++;
		if (timeout > 50000) { 
			kputs("ppa: timeout.\n");
			return 0;
		}
	}
	return r & 0xF0;
}

/*
 *	Although it's done by bashing a parallel port the basics
 *	are actually much the same as many other SCSI controllers
 *
 *	Select the device
 *	Read or write according to phase returned
 */
static uint8_t ppa_command(uint8_t target, uint8_t *cmd, uint8_t cmd_len, uint8_t *data, uint8_t block)
{
	uint8_t d;
	uint8_t i;
	uint8_t r;

	ppa_connect();
	if (ppa_select(7, target) == 0)
		return 0xFF;

	ppa_write_ctrl(0x0C);
	for (i = 0; i < cmd_len;i++) {
		if (!ppa_wait_done())
			return 0xFF;
		ppa_write_data(*cmd++);
		ppa_write_ctrl(0x0E);
		ppa_write_ctrl(0x0C);
	}
	/* Ok command clocked out */
	while(1) {
		r = ppa_wait_done();
		switch(r) {
		/* SCSI bus state */
		case 0xC0:	/* Write */
		        if (block)
		            ppa_block_write();
                        else {
			    ppa_write_data(*data++);
			    ppa_write_ctrl(0x0E);
			    ppa_write_ctrl(0x0C);
                        }
			break;
		case 0xD0:	/* Read */
		        if (block)
		            ppa_block_read();
                        else {
			    ppa_write_ctrl(0x04);
			    d = ppa_read_status() & 0xF0;
			    ppa_write_ctrl(0x06);
			    d |= ppa_read_status() >> 4;
			    /* TODO : length check */
			    *data++ = d;
			    ppa_write_ctrl(0x0C);
                        }
			break;
		/* E0 appears to be command so we shouldn;t end up back in it */
		case 0xF0:	/* Status */
			ppa_write_ctrl(0x04);
			d = ppa_read_status() & 0xF0;
			ppa_write_ctrl(0x06);
			d |= ppa_read_status() >> 4;
			ppa_write_ctrl(0x0E);
			ppa_write_ctrl(0x0C);
			ppa_disconnect();
			return d;
		default:
			kprintf("ppa: bad status %2x\n", r);
			return 0xFF;
		}
	}
}		

static uint8_t cmd_rw[6] = { 0x00, 0x00, 0x00, 0x00, 0x01, 0x00 };

static uint_fast8_t ppa_transfer_sector(void)
{
    cmd_rw[0] = blk_op.is_read ? 0x08 : 0x0A;
    cmd_rw[1] = (blk_op.lba >> 16) & 0x0F;
    cmd_rw[2] = blk_op.lba >> 8;
    cmd_rw[3] = blk_op.lba;
    if (ppa_command(6, cmd_rw, 6, blk_op.addr, 1))
        return 0;
    return 1;
}

static int ppa_flush_cache(void)
{
	return 0;
}

static uint8_t ppa_devinit(void)
{
	uint8_t s;
	ppa_write_data(0xAA);
	ppa_disconnect();
	ppa_connect();
	ppa_write_ctrl(0x06);
	if ((ppa_read_status() & 0xF0) != 0xF0)
		return 2;
	ppa_write_ctrl(0x04);
	if ((ppa_read_status() & 0xF0) != 0x80)
		return 3;
	ppa_disconnect();
	s = 0x0C;	/* Check what ppa_disconect left */
	ppa_write_data(0xAA);
	ppa_write_ctrl(s);	
	ppa_connect();
	ppa_write_data(0x40);
	ppa_write_ctrl(0x08);
	ppa_write_ctrl(0x0C);
	ppa_disconnect();
	return 0;
}

uint8_t ppa_init(void)
{
    blkdev_t *blk;

    if (ppa_devinit())
        return 0;
    
    blk = blkdev_alloc();
    if (blk == NULL)
        return 0;
    blk->transfer = ppa_transfer_sector;
    blk->flush = ppa_flush_cache;
    blk->driver_data = 0;
    blk->drive_lba_count = 0xFFFFFF;	 /* For now */
    kputs("ZIP drive 0: ");
    blkdev_scan(blk, 1);
    return 1;
}

#endif
