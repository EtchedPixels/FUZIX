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
       platform_idle();
}

static uint8_t sunrise_transfer_sector(void)
{
    uint8_t drive = (blk_op.blkdev->driver_data & IDE_DRIVE_NR_MASK);
    uint8_t mask = drive ? 0xF0 : 0xE0;
    uint8_t *addr = blk_op.addr;
    uint8_t old_user = blk_op.is_user;

    if (!blk_op.is_read)
        blk_op.blkdev->driver_data |= FLAG_CACHE_DIRTY;
    /* Shortcut: this range can only occur for a user mode I/O */
    if (addr >= (uint8_t *)0x3E00U && addr < (uint8_t *)0x8000U) {
        /* We can't just use tmpbuf because the buffer might be dirty which would
           trigger a recursive I/O and then badness happens */
        blk_op.addr = bouncebuffer;
        blk_op.is_user = 0;
//        kprintf("bounced do_ide_xfer %p %x:", addr, mask);
        if (blk_op.is_read) {
            if (do_ide_xfer(mask))
                goto fail;
            uput(blk_op.addr, addr, 512);
        } else {
            uget(addr, blk_op.addr, 512);
            if (do_ide_xfer(mask))
                goto fail;
        }
//        kprintf("bounced done.\n");
        blk_op.addr = addr;
        blk_op.is_user = old_user;
        return 1;
    }
//    kprintf("do_ide_xfer %d %p %x..", blk_op.is_user, addr, mask);
    if (do_ide_xfer(mask) == 0) {
//        kputs("done.\n");
        return 1;
    }
fail:
    blk_op.addr = addr;
    blk_op.is_user = old_user;
    if (ide_error == 0xFF)
        kprintf("ide%d: timeout.\n", drive);
    else
        kprintf("ide%d: status %x\n", drive, ide_error);
    return 0;
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

static const uint16_t sunrise_roms[2] = {
    0x8FB3,
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
    memcpy(&sunrise_u, map_slot1_user(i), sizeof(sunrise_k));

    kprintf("sunrise_k: %x %x %x %x %x %x\n",
        sunrise_k.private[0], sunrise_k.private[1], sunrise_k.private[2],
        sunrise_k.private[3], sunrise_k.private[4], sunrise_k.private[5]);

    do_ide_begin_reset();
    delay();
    do_ide_end_reset();
    delay();
    for (i = 0; i < IDE_DRIVE_COUNT; i++)
        sunrise_init_drive(i);
}
