/*
 *	XMS Ramdisk Driver.
 *
 *	This allows us to use high memory on a 286 system as a RAM disk
 *
 *	FIXME: make this use the block stuff and partitions
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devhd.h>
#include <bios.h>

static uint16_t xms_size;

static int xm_transfer(bool is_read, uint8_t rawflag)
{
    uint16_t page = kernel_ds;
    uint32_t dptr;

    if (rawflag == 1) {
        if (d_blkoff(9))
            return -1;
        page = get_segment(udata.u_page);
#ifdef SWAPDEV        
    } else if (rawflag == 2)
        page = swappage;
#endif

    /* dptr is effectively a far pointer */
    dptr = (uint32_t)udata.u_dptr;
    dptr += page << 4;

    /* Should we add a simple length check to the core disk I/O ? */
    if (udata.u_block + udata.u_nblock > xms_size ||
        udata.u_block + udata.u_nblock < udata.u_block)
    {
        udata.u_error = EINVAL;
        return -1;
    }
    if (is_read) {
        os = gdt + 0x20;
        disk = gdt + 0x30;
    else {
        os = gdt + 0x30;
        disk = gdt + 0x20;
    }
    
    /*
       FIXME: we should loop this with a limited transfer size because
       1. We can't do > 32k words at a time (not clear if can do exactly 32k)
       2. It runs with IRQs off
     */
    /* Turn the disk block into a 24 bit address remembering we start at 1MB */
    disk[2] = 0x00;				/* A0-A7 */
    disk[3] = udata.u_block << 1;		/* A8-A15 */
    disk[4] = (udata.u_block >> 7) + 16;	/* A16-A23 */
    /* Turn the native far address into a 24bit address */
    os[2] = dptr;
    os[3] = dptr >> 8;
    os[4] = dptr >> 16;
    /* The other bytes are pre-filled */
    /* Length is in words */
    err = xmm_xfer(udata.u_nblock << 8);
    if (err) {
        /* 0x01 - parity, 0x02 exception, 0x03 gate a20 failed */
        kprintf("xm0: error %d\n", err);
        udata.u_error = EIO;
        return -1;
    }
    return udata.u_nblock << 9;
}

int xm_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    return xm_transfer(true, rawflag);
}

int xm_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    return xm_transfer(false, rawflag);
}

int xm_open(uint8_t minor, uint16_t flag)
{
    if (minor || !xms_size)
        return ENODEV;
    return 0;
}

void xm_init(void)
{
    if (xm_lockout)
        return;
    xms_size = xmm_probe();
    if (xms_size)
        kprintf("xm0: %dKb extended memory RAMdisc\n", xms_size);
    xms_size *= 2;		/* Blocks */
}

/* Certain 286 systems can make memory visible as both extended and expanded.
   On those we need to lock out the less useful extended form */
void xm_disable(void)
{
    xm_lockout = 1;
}
