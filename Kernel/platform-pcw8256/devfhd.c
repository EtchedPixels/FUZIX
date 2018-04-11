/*
 *	An implementation of the FID based hard disk interface for Joyce.
 *	Currently only hard disk interfaces are handled by this driver.
 */
#include <kernel.h>
#include <kdata.h>
#include <blkdev.h>
#include <devfhd.h>
#include <printf.h>

#define MAX_FHD	16

static const char *errstr[2] = { "write", "read" };
static uint8_t drivemask;
static uint8_t drivebase;
static struct dpb dpb[MAX_FHD];

static uint8_t devfhd_transfer_sector(void)
{
    uint8_t err;
    uint8_t drive = blk_op.blkdev->driver_data;
    fhd_drive = drivebase + drive;
    fhd_dpb = dpb + drive;
    /* For now assume HD fixed geometry */
    fhd_track = blk_op.lba >> 5;
    fhd_sector = blk_op.lba & 0x1F;
    fhd_op = blk_op.is_read ? 4 : 5;
    err = rw_fidhd();
    if (err) {
        kprintf("fhd%c: %s error %d\n", 'a' + drive, errstr[blk_op.is_read], err);
        return 0;
    }
    return 1;
}

static int devfhd_flush_cache(void)
{
    uint8_t err;
    uint8_t drive = blk_op.blkdev->driver_data;
    fhd_drive = drivebase + drive;
    fhd_dpb = dpb + drive;
    err = flush_fidhd();
    if (err) {
        kprintf("fhd%c: flush error %d\n", 'a' + drive, err);
        udata.u_error = EIO;
        return -1;
    }
    return 0;
}

/* FIXME These bits could be discard.. */

int devfhd_init(void)
{
    blkdev_t *blk;
    uint8_t d;

    /* Either not supported or too old */
    if (probe_fidhd() < 2)
        return 0;
    fhd_op = 4;
    fhd_drive = 0;
    fhd_dpb = dpb;
    /* Find first slot to install */
    fhd_drive = drivebase = install_fidhd();
    if (fhd_drive == 0xFF)
        return 0;
    /* Now begin registering */
    fhd_op = 0;
    for (d = 0; d < MAX_FHD; d++) {
        uint8_t next = install_fidhd();
        if (next != 0xFF)
            drivemask |= (1 << d);
        /* The current driver guarantees us a 1:1 so we don't do a lookup
           table. Maybe we should for the future and use next as intended */
        fhd_drive++;
        fhd_dpb++;
    }
    fhd_dpb = dpb;
    fhd_drive = drivebase;
    for (d = 0; d + drivebase < MAX_FHD; d++) {
        if (drivemask & (1 << d)) {
            login_fidhd();
            blk = blkdev_alloc();
            if (!blk)
                break;
            blk->transfer = devfhd_transfer_sector;
            blk->flush = devfhd_flush_cache;
            blk->driver_data = d;
            blk->drive_lba_count = 16834;	/* FIXME use dpb */
            blkdev_scan(blk, SWAPSCAN);	/* Won't damage fhd_dpb/drive */
        }
        fhd_drive++;
        fhd_dpb++;
    }
    return drivemask;
}
