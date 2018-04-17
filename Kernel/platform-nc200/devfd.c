/* 
 *	Amstrad PCW8256 Floppy Driver
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devfd.h>
#include <device.h>
#include <blkdev.h>

bool fd765_ready;

static uint8_t motorct;
static int8_t devsel = -1;

__sfr __at 0xe0 fd_st;

void devfd_init(void)
{
    blkdev_t* blk = blkdev_alloc();
    if (!blk)
        return;

    fd765_track = 0xff; /* not on a known track */
    blk->transfer = devfd_transfer;
    blk->drive_lba_count = 1440; /* 512-byte sectors */
    blkdev_scan(blk, 0);
}

void devfd_spindown(void)
{
}

static void motor_on(int minor)
{
    minor;
    mod_control(0x00, 0x20); /* motor on (active low) */
    fd765_ready = false;
    mod_irqen(0x20, 0x00); /* enable FDC IRQ */
}

static void motor_off(void)
{
    mod_control(0x20, 0x00); /* motor off (active low( */
    mod_irqen(0x00, 0x20); /* disable FDC IRQ */
    devsel = -1;
}

/* Seek to track 0. */
static void fd_recalibrate(void)
{
    /* Keep trying to recalibrate until the command succeeds. We use this
     * as a really shoddy way to wait for spinup. There should be a timeout here.
     */

    for (;;)
    {
        fd765_do_recalibrate();
        if (((fd765_status[0] & 0xf8) == 0x20) && !fd765_status[1])
            break;
    }

    fd765_track = 0;
}

/* Set up the controller for a given block, seek, and wait for spinup. */
static void fd_seek(void)
{
    uint16_t block = blk_op.lba;
    uint8_t track2 = block / 9;
    uint8_t newtrack = track2 >> 1;

    fd765_sector = (block % 9) + 1;
    fd765_head = track2 & 1;

    if (newtrack != fd765_track)
    {
        fd765_track = newtrack;

        for (;;)
        {
            fd765_do_nudge_tc();
            fd765_do_seek();
            if ((fd765_status[0] & 0xf8) == 0x20)
                break;

            fd_recalibrate();
        }
    }
}

/* Select a drive and ensure the motor is on. */
static void fd_select(int minor)
{
    if (devsel == minor)
        return;
    motor_on(minor);
}

/*
 *	Block transfer
 */
uint8_t devfd_transfer(void)
{
    int ct = 0;
    int tries;

    fd_select(0);		/* Select, motor on */
    fd765_is_user = blk_op.is_user;
    while (ct < blk_op.nblock) {	/* For each block */
        for (tries = 0; tries < 3; tries ++) {	/* Try 3 times */
            if (tries != 0)
                fd_recalibrate();
            fd_seek();

            fd765_buffer = blk_op.addr;
            if (blk_op.is_read) {
                fd765_do_read();
            } else {
                fd765_do_write();
            }

            /* Did it work ? */
            if ((fd765_status[0] & 0xc0) == 0)
                break;
        }
        if (tries == 3)
        {
            kprintf("fd%d: I/O error %d:%d\n", blk_op.is_read, blk_op.lba);
            udata.u_error = EIO;
            break;
        }
        udata.u_block++;
        ct++;
        blk_op.addr += 512;
    }
    return ct;
}