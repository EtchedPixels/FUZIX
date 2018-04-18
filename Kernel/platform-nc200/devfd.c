#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devfd.h>
#include <device.h>
#include <blkdev.h>
#include <timer.h>

static timer_t spindown_timer = 0;

void devfd_init(void)
{
    blkdev_t* blk = blkdev_alloc();
    if (!blk)
        return;

    fd765_do_nudge_tc();
    fd765_track = 0xff; /* not on a known track */
    blk->transfer = devfd_transfer;
    blk->drive_lba_count = 1440; /* 512-byte sectors */
    blkdev_scan(blk, 0);
}

static void nudge_timer(void)
{
    di();
    spindown_timer = set_timer_sec(2);
    ei();
}

/* (called from interrupt context) */
void devfd_spindown(void)
{
    if (spindown_timer && timer_expired(spindown_timer))
    {
        mod_control(0x20, 0x00); /* motor off (active low) */
        spindown_timer = 0;
    }
}

/* Seek to track 0. */
static void fd_recalibrate(void)
{
    /* Keep trying to recalibrate until the command succeeds. We use this
     * as a really shoddy way to wait for spinup. There should be a timeout here.
     */

    for (;;)
    {
        nudge_timer();
        fd765_do_recalibrate();
        if (((fd765_status[0] & 0xf8) == 0x20) && !fd765_status[1])
            break;
    }

    /* Forget which track we've saught to */
    fd765_track = 0xff;
}

/* Set up the controller for a given block, seek, and wait for spinup. */
static void fd_seek(uint16_t lba)
{
    uint8_t track2 = lba / 9;
    uint8_t newtrack = track2 >> 1;

    fd765_sector = (lba % 9) + 1;
    fd765_head = track2 & 1;

    if (newtrack != fd765_track)
    {
        for (;;)
        {
            fd765_track = newtrack;
            nudge_timer();
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
    (void)minor;

    mod_control(0x00, 0x20); /* motor on (active low) */
    nudge_timer();
}

uint8_t devfd_transfer(void)
{
    int ct = 0;
    int tries;
    int blocks = blk_op.nblock;
    uint16_t lba = blk_op.lba;

    fd_select(0);
    fd765_is_user = blk_op.is_user;
    fd765_buffer = blk_op.addr;

    while (blocks != 0)
    {
        for (tries = 0; tries < 3; tries ++)
        {
            nudge_timer();
            if (tries != 0)
                fd_recalibrate();
            fd_seek(lba);

            fd765_sectors = 10 - fd765_sector;
            if (fd765_sectors > blocks)
                fd765_sectors = blocks;

            if (blk_op.is_read)
                fd765_do_read();
            else
                fd765_do_write();

            /* Did it work ? */
            if ((fd765_status[0] & 0xc0) == 0)
                break;
        }
        if (tries == 3)
        {
            kprintf("fd%d: I/O error %d:%d\n", blk_op.is_read, lba);
            udata.u_error = EIO;
            break;
        }
        lba += fd765_sectors;
        blocks -= fd765_sectors;
        ct += fd765_sectors;
    }

    return ct;
}