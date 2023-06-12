#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <blkdev.h>
#include <devide_sunrise.h>
#include <msx2.h>

extern uint16_t ide_error;
extern uint16_t ide_base;
extern uint8_t ide_slot;
extern uint16_t ide_addr;
extern uint32_t ide_lba;
extern uint8_t ide_is_read;
uint8_t *devide_buf;

static void delay(void)
{
    timer_t timeout;

    timeout = set_timer_ms(25);

    while(!timer_expired(timeout))
       plt_idle();
}

/* We assume that memory at 0xc000-0xc1fe is not allocated
 * to solve the issue of page crossing below.
 * A sector is read, and any surplus is copied to the next page.
 * A sector is written after any surplus is copied from the next page.
 */
static uint8_t sunrise_transfer_sector(void)
{
    uint8_t drive = (blk_op.blkdev->driver_data & IDE_DRIVE_NR_MASK);
    uint8_t mask = drive ? 0xF0 : 0xE0;
    uint8_t *page, page_offset, *addr = blk_op.addr;
    uint16_t status;

    ide_lba = blk_op.lba;
    ide_is_read = blk_op.is_read;

    if (!ide_is_read)
        blk_op.blkdev->driver_data |= FLAG_CACHE_DIRTY;

    irqflags_t irq = di();
    if (blk_op.is_user) {
        ide_addr = ((uint16_t) addr) % 0x4000 + 0x8000;
        page_offset = (((uint16_t)addr) / 0x4000);
        page = &udata.u_page;
        int len = ide_addr + BLKSIZE - 0xc000;
        if (len > 0 && !ide_is_read) {
            RAM_PAGE2 = *(page + page_offset + 1);
            memcpy((uint8_t *)0xc000, (uint8_t *)0x8000, len);
        }
        RAM_PAGE2 = *(page + page_offset);
        status = do_ide_xfer(mask);
        if (len > 0 && ide_is_read) {
            RAM_PAGE2 = *(page + page_offset + 1);
            memcpy((uint8_t *)0x8000, (uint8_t *)0xc000, len);
        }
        RAM_PAGE2 = 1;
    } else {
        ide_addr = (uint16_t) addr;
        status = do_ide_xfer(mask);
    }
    irqrestore(irq);

    if (status != 0) {
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

/* byte sequence at 0x4098 to identify Sunrise IDE ROM */
static const char sunrise_id[] = "\x40\x7e\x32\x04\x41\xf1\xe1\xc9";

void sunrise_probe(void)
{
    uint8_t i;

    ide_base = 0x7E00;

    for (ide_slot = 1; ide_slot < 3; ide_slot++) {
        mapslot_bank1(ide_slot);
        if (memcmp(0x4098, sunrise_id, 8) == 0)
            break;
    }
    mapslot_bank1(slotram);
    if (ide_slot == 3)
        return;

    kprintf("Sunrise IDE found in slot %d\n", ide_slot);

    do_ide_begin_reset();
    delay();
    do_ide_end_reset();
    delay();
    for (i = 0; i < IDE_DRIVE_COUNT; i++)
        sunrise_init_drive(i);
}
