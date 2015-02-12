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

/****************************************************************************/
/* Code below this point used only once, at startup, so we want it to live  */
/* in the DISCARD segment. sdcc only allows us to specify one segment for   */
/* each source file. This "solution" is a bit (well, very) hacky ...        */
/****************************************************************************/

static void devide_delay(void)
{
    timer_t timeout;

    timeout = set_timer_ms(25);

    while(!timer_expired(timeout))
       platform_idle();
}

void devide_init_drive(uint8_t drive)
{
    blkdev_t *blk;
    uint8_t *buffer, select;

    switch(drive){
	case 0: select = 0xE0; break;
	case 1: select = 0xF0; break;
        default: return;
    }

    kprintf("IDE drive %d: ", drive);

    /* Reset depends upon the presence of alt control, which is optional */
#ifdef IDE_REG_CONTROL
    /* reset the drive */
    ide_reg_devhead = select;
    ide_reg_control = 0x06; /* assert reset, no interrupts */
    devide_delay();
    ide_reg_control = 0x02; /* release reset, no interrupts */
    devide_delay();
    if(!devide_wait(IDE_STATUS_READY))
        return;
#endif

#ifdef IDE_8BIT_ONLY
    /* set 8-bit mode -- mostly only supported by CF cards */
    ide_reg_devhead = select;
    if(!devide_wait(IDE_STATUS_READY))
        return;
    ide_reg_features = 0x01;
    ide_reg_command = IDE_CMD_SET_FEATURES;
#endif

    /* confirm drive has LBA support */
    if(!devide_wait(IDE_STATUS_READY))
        return;

    /* send identify command */
    ide_reg_devhead = select;
    ide_reg_command = IDE_CMD_IDENTIFY;

    /* allocate temporary sector buffer memory */
    buffer = (uint8_t *)tmpbuf();

    if(!devide_wait(IDE_STATUS_DATAREQUEST))
	goto failout;

    blk_op.is_user = false;
    blk_op.addr = buffer;
    blk_op.nblock = 1;
    devide_read_data();

    if(!(buffer[99] & 0x02)){
        kputs("LBA unsupported.\n");
        goto failout;
    }

    blk = blkdev_alloc();
    if(!blk)
	goto failout;

    kputs("BAL");
    blk->transfer = devide_transfer_sector;
    blk->flush = devide_flush_cache;
    blk->driver_data = drive & DRIVE_NR_MASK;

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
    brelse((bufptr)buffer);

    /* scan partitions */
    blkdev_scan(blk, SWAPSCAN);

    return;
failout:
    brelse((bufptr)buffer);
    return;
}

void devide_init(void)
{
    uint8_t d;

    for(d=0; d<DRIVE_COUNT; d++)
        devide_init_drive(d);
}
