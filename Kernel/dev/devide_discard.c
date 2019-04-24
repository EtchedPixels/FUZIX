/*-----------------------------------------------------------------------*/
/* IDE interface driver                                                  */
/* 2014-11-02 Will Sowerbutts (unreleased UZI-mark4)                     */
/* 2014-12-22 WRS ported to Fuzix                                        */
/* 2014-12-25 WRS updated to also support P112 GIDE                      */
/* 2015-01-04 WRS updated to new blkdev API                              */
/* 2015-01-25 WRS updated to newer blkdev API                            */
/*-----------------------------------------------------------------------*/

#define _IDE_PRIVATE

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <timer.h>
#include <devide.h>
#include <blkdev.h>

#ifdef CONFIG_IDE

/****************************************************************************/
/* Code in this file is used only once, at startup, so we want it to live   */
/* in the DISCARD segment. sdcc only allows us to specify one segment for   */
/* each source file.                                                        */
/****************************************************************************/

#ifdef IDE_REG_CONTROL

#define IDE_HAS_RESET

static void devide_delay(void)
{
    timer_t timeout;

    timeout = set_timer_ms(25);

    while(!timer_expired(timeout))
       platform_idle();
}

/* Reset depends upon the presence of alt control, which is optional */
void devide_reset(void)
{
    kputs("IDE reset\n");

    /* reset both drives */
    devide_writeb(ide_reg_devhead, 0xE0); /* select master */
    devide_writeb(ide_reg_control, 0x06); /* assert reset, no interrupts */
    devide_delay();

    devide_writeb(ide_reg_control, 0x02); /* release reset, no interrupts */
    devide_delay();
}
#endif

void devide_init_drive(uint_fast8_t drive)
{
    blkdev_t *blk;
    uint8_t *buffer;
    uint_fast8_t select;

    select = (drive & 1) ? 0xF0 : 0xE0;

    ide_select(drive);

    devide_writeb(ide_reg_devhead, select);
    kprintf("IDE drive %d: ", drive);

#ifdef IDE_8BIT_ONLY
    if (IDE_IS_8BIT(drive)) {
    /* set 8-bit mode -- mostly only supported by CF cards */
        if (!devide_wait(IDE_STATUS_READY))
            goto out;

        devide_writeb(ide_reg_devhead, select);
        if (!devide_wait(IDE_STATUS_READY))
            goto out;

        devide_writeb(ide_reg_features, 0x01); /* Enable 8-bit PIO transfer mode (CFA feature set only) */
        devide_writeb(ide_reg_command, IDE_CMD_SET_FEATURES);
    }
#endif

    /* confirm drive has LBA support */
    if (!devide_wait(IDE_STATUS_READY))
        goto out;

    /* send identify command */
    devide_writeb(ide_reg_devhead, select);
    devide_writeb(ide_reg_command, IDE_CMD_IDENTIFY);

    /* allocate temporary sector buffer memory */
    buffer = (uint8_t *)tmpbuf();

    if (!devide_wait(IDE_STATUS_DATAREQUEST))
	goto failout;

    blk_op.is_user = false;
    blk_op.addr = buffer;
    blk_op.nblock = 1;
    devide_read_data();

#ifdef CONFIG_IDE_BSWAP
    if(!(buffer[98] & 0x02)) {
#else
    if(!(buffer[99] & 0x02)) {
#endif    
        kputs("LBA unsupported.\n");
        goto failout;
    }

    blk = blkdev_alloc();
    if(!blk)
	goto failout;

    blk->transfer = devide_transfer_sector;
    blk->flush = devide_flush_cache;
    blk->driver_data = drive & IDE_DRIVE_NR_MASK;

    if( !(((uint16_t*)buffer)[82] == 0x0000 && ((uint16_t*)buffer)[83] == 0x0000) ||
         (((uint16_t*)buffer)[82] == 0xFFFF && ((uint16_t*)buffer)[83] == 0xFFFF) ){
	/* command set notification is supported */
	if(buffer[164] & 0x20){
	    /* write cache is supported */
            blk->driver_data |= FLAG_WRITE_CACHE;
	}
    }

    /* read out the drive's sector count */
    blk->drive_lba_count = le32_to_cpu(*((uint32_t*)&buffer[120]));

    /* done with our temporary memory */
    tmpfree(buffer);

    /* Deselect the IDE, as we will re-select it in the partition scan and
       it may not recursively stack de-selections */
    ide_deselect();

    /* scan partitions */
    blkdev_scan(blk, SWAPSCAN);

    return;
failout:
    tmpfree(buffer);
out:
    ide_deselect();
    return;
}

void devide_init(void)
{
    uint_fast8_t d;

#ifdef IDE_HAS_RESET
    devide_reset();
#endif

    for(d=0; d < IDE_DRIVE_COUNT; d++)
        devide_init_drive(d);
}

#endif /* CONFIG_IDE */
