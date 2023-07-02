#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devdw.h>
#include <drivewire.h>

#define DW_READ		0
#define DW_WRITE	1

/*
 *	Block device glue for DriveWire
 *
 *	DriveWire uses 256 byte sector transfers
 */

static int dw_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
    int ct = 0;
    int tries;
    uint8_t err;
    uint8_t cmd[9];
    uint16_t page = 0;
    irqflags_t irq;

    /* FIXME: allow raw 256 byte transfers */

    if (rawflag == 1) {
        if (d_blkoff(BLKSHIFT))
            return -1;
        page = udata.u_page;
    } else if (rawflag == 2) {
#if defined(SWAPDEV) || defined(PAGEDEV)
        page = (uint16_t)swappage;
#else
	goto bad2;
#endif
    }

    udata.u_nblock *= 2;	/* 256 bytes/block */

/*    kprintf("Issue command: drive %d\n", minor); */
    /* maybe mimicking floppy driver more than needed? */
    cmd[0] = is_read ? DW_READ : DW_WRITE;
    cmd[1] = udata.u_block >> 7;	/* 2 sectors per block */
    cmd[2] = (udata.u_block << 1) & 0xFF;
    cmd[3] = (uint16_t)udata.u_dptr >> 8;
    cmd[8] = (udata.u_block & 0x8000) ? 1 : 0;
    cmd[4] = (uint16_t)udata.u_dptr & 0xFF;
    cmd[5] = minor;
    cmd[6] = page >> 8;
    cmd[7] = page & 0xFF;
        
    while (ct++ < udata.u_nblock) {
        for (tries = 0; tries < 4 ; tries++) {
            /* kprintf("dw_operation block %d left %d\n", block, nblock); */
            irq = di(); /* for now block interrupts for whole operation */
            err = dw_operation(cmd);
            irqrestore(irq);
            if (err == 0)
                break;
            if (tries > 1)
                dw_reset();
        }
        /* FIXME: should we try the other half and then bail out ? */
        if (tries == 4)
            goto bad;
        cmd[3]++;	/* Move on 256 bytes in the buffer */
        cmd[2]++;	/* Next sector for 2nd block */
        if (cmd[2] == 0) {
            cmd[1]++;
	    if (cmd[1] == 0)
		cmd[8]++;
	}
    }
    return udata.u_nblock << 8;
bad:
    kprintf("dw%d: error %x\n", minor, err);
bad2:
    udata.u_error = EIO;
    return -1;
}

/* FIXME: for bit-banger transport (not Becker) we should set up
   the PIA at some point too */

int dw_open(uint8_t minor, uint16_t flag)
{
    used(flag);
    used(minor);
    return 0;
}

int dw_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    used(flag);
    return dw_transfer(minor, true, rawflag);
}

int dw_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    used(flag);
    return dw_transfer(minor, false, rawflag);
}

int dw_ioctl(uint8_t minor, uarg_t request, char *data)
{
	struct dw_trans s;
	used( minor );

	if (request != DRIVEWIREC_TRANS)
		return -1;
	if (uget( data, &s, sizeof(struct dw_trans)))
		return -1;

	if (dw_transaction(s.sbuf, s.sbufz, s.rbuf, s.rbufz, 1)) {
		udata.u_error = EIO;
		return -1;
        }
        return 0;
}
