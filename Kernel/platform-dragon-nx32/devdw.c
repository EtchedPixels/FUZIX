#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devdw.h>

#define MAX_DW	4	/* can be 255 */

#define DW_READ		0
#define DW_WRITE	1

static uint8_t dw_tab[MAX_DW];

/*
 *	Block device glue for DriveWire
 *
 *	DriveWire uses 256 byte sector transfers
 */

static int dw_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
    blkno_t block;
    uint16_t dptr;
    int nblock;
    int tries;
    uint8_t err;
    uint8_t *driveptr = dw_tab + minor;
    uint8_t cmd[6];
    irqflags_t irq;

    if (rawflag == 0)  {
        dptr = (uint16_t)udata.u_buf->bf_data;
        block = udata.u_buf->bf_blk;
        nblock = 2;
    } else if (rawflag == 1) {
        if (((uint16_t)udata.u_offset|udata.u_count) & BLKMASK)
            goto bad2;
        dptr = (uint16_t)udata.u_base;
        block = udata.u_offset >> 9;
        nblock = udata.u_count >> 8;
    } else
        goto bad2;


//    kprintf("Issue command: drive %d\n", minor);
    /* maybe mimicking floppy driver more than needed? */
    cmd[0] = is_read ? DW_READ : DW_WRITE;
    cmd[1] = block >> 7;	/* 2 sectors per block */
    cmd[2] = (block << 1) & 0xFF;
    cmd[3] = dptr >> 8;
    cmd[4] = dptr & 0xFF;
    cmd[5] = rawflag;
    *driveptr = minor; /* pass minor (drive number) through here for now */
        
    while (nblock--) {
        for (tries = 0; tries < 4 ; tries++) {
            // kprintf("dw_operation block %d left %d\n", block, nblock);
            irq = di(); /* for now block interrupts for whole operation */
            err = dw_operation(cmd, driveptr);
            irqrestore(irq);
            if (err == 0)
                break;
            if (tries > 1)
                dw_reset(driveptr);
        }
        /* FIXME: should we try the other half and then bail out ? */
        if (tries == 3)
            goto bad;
        cmd[3]++;	/* Move on 256 bytes in the buffer */
        cmd[2]++;	/* Next sector for 2nd block */
        if (cmd[2] == 0)
            cmd[1]++;
    }
    return 1;
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
    if(minor >= MAX_DW) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int dw_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    return dw_transfer(minor, true, rawflag);
}

int dw_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    return dw_transfer(minor, false, rawflag);
}

