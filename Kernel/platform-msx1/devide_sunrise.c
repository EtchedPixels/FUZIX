#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <blkdev.h>
#include <devide_sunrise.h>
#include <msx.h>

uint16_t ide_error;
uint16_t ide_base = 0x7E00;
uint8_t *devide_buf;

struct msx_map sunrise_u, sunrise_k;

static void delay(void)
{
    timer_t timeout;

    timeout = set_timer_ms(25);

    while(!timer_expired(timeout))
       plt_idle();
}

static uint8_t sunrise_transfer_sector(void)
{
    uint8_t drive = (blk_op.blkdev->driver_data & IDE_DRIVE_NR_MASK);
    uint8_t mask = drive ? 0xF0 : 0xE0;

    if (!blk_op.is_read)
        blk_op.blkdev->driver_data |= FLAG_CACHE_DIRTY;
    if (blk_xfer_bounced(do_ide_xfer, mask) == 0) {
        if (ide_error == 0xFF)
            kprintf("ide%d: timeout.\n", drive);
        else
            kprintf("ide%d: status %x\n", drive, ide_error);
        return 0;
    }
    return 1;
}        
    
static int sunrise_flush_cache(void)
{
    uint8_t drive;

    drive = blk_op.blkdev->driver_data & IDE_DRIVE_NR_MASK;
    /* check drive has a cache and was written to since the last flush */
    if((blk_op.blkdev->driver_data & (FLAG_WRITE_CACHE | FLAG_CACHE_DIRTY))
		                 != (FLAG_WRITE_CACHE | FLAG_CACHE_DIRTY))
        return 0;
    
    if (do_ide_flush_cache(drive ? 0xF0 : 0xE0)) {
        udata.u_error = EIO;
        return -1;
    }
    return 0;
}

static void sunrise_init_drive(uint8_t drive)
{
    uint8_t mask = drive ? 0xF0 : 0xE0;
    blkdev_t *blk;

    kprintf("IDE drive %d: ", drive);
    devide_buf = tmpbuf();
    if (do_ide_init_drive(mask) == NULL)
        goto failout;
    if (!(devide_buf[99] & 0x02)) {
        kputs("LBA unsupported.\n");
        goto failout;
    }
    blk = blkdev_alloc();
    if (!blk)
        goto failout;
    blk->transfer = sunrise_transfer_sector;
    blk->flush = sunrise_flush_cache;
    blk->driver_data = drive;

    if( !(((uint16_t*)devide_buf)[82] == 0x0000 && ((uint16_t*)devide_buf)[83] == 0x0000) ||
         (((uint16_t*)devide_buf)[82] == 0xFFFF && ((uint16_t*)devide_buf)[83] == 0xFFFF) ){
	/* command set notification is supported */
	if(devide_buf[164] & 0x20){
	    /* write cache is supported */
            blk->driver_data |= FLAG_WRITE_CACHE;
	}
    }

    /* read out the drive's sector count */
    blk->drive_lba_count = le32_to_cpu(*((uint32_t*)&devide_buf[120]));

    /* done with our temporary memory */
    tmpfree(devide_buf);
    blkdev_scan(blk, SWAPSCAN);
    return;
failout:
    kputs("\n");
    tmpfree(devide_buf);
}

static const uint16_t sunrise_roms[4] = {
    0x8FB3,
    0xBB0E,
    0xABF2,
    0x0000
};

void sunrise_probe(void)
{
    uint8_t slot, subslot;
    uint8_t i;

    i = device_find(sunrise_roms);
    if (i == 0xFF)
        return;

    /* Generate and cache the needed mapping table */
    memcpy(&sunrise_k, map_slot1_kernel(i), sizeof(sunrise_k));
    memcpy(&sunrise_u, map_slot1_user(i), sizeof(sunrise_u));
#define DEBUG
#ifdef DEBUG
    kprintf("sunrise_k: %2x %2x %2x %2x %2x %2x\n",
        sunrise_k.private[0], sunrise_k.private[1], sunrise_k.private[2],
        sunrise_k.private[3], sunrise_k.private[4], sunrise_k.private[5]);
    kprintf("sunrise_u: %2x %2x %2x %2x %2x %2x\n",
        sunrise_u.private[0], sunrise_u.private[1], sunrise_u.private[2],
        sunrise_u.private[3], sunrise_u.private[4], sunrise_u.private[5]);
#endif

    do_ide_begin_reset();
    delay();
    do_ide_end_reset();
    delay();
    for (i = 0; i < IDE_DRIVE_COUNT; i++)
        sunrise_init_drive(i);
}
