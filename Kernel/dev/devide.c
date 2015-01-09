/*-----------------------------------------------------------------------*/
/* IDE interface driver                                                  */
/* 2014-11-02 Will Sowerbutts (unreleased UZI-mark4)                     */
/* 2014-12-22 WRS ported to Fuzix                                        */
/* 2014-12-25 WRS updated to also support P112 GIDE                      */
/* 2015-01-04 WRS updated to new blkdev API                              */
/*-----------------------------------------------------------------------*/

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <timer.h>
#include <devide.h>
#include <blkdev.h>

#define DRIVE_COUNT 2   /* range 1 -- 4 */

static uint8_t drive_flags[DRIVE_COUNT];
#define FLAG_WRITE_CACHE 1

#ifdef IDE_REG_ALTSTATUS
__sfr __at IDE_REG_ALTSTATUS ide_reg_altstatus;
#endif
#ifdef IDE_REG_CONTROL
__sfr __at IDE_REG_CONTROL   ide_reg_control;
#endif
__sfr __at IDE_REG_COMMAND   ide_reg_command;
__sfr __at IDE_REG_DATA      ide_reg_data;
__sfr __at IDE_REG_DEVHEAD   ide_reg_devhead;
__sfr __at IDE_REG_ERROR     ide_reg_error;
__sfr __at IDE_REG_FEATURES  ide_reg_features;
__sfr __at IDE_REG_LBA_0     ide_reg_lba_0;
__sfr __at IDE_REG_LBA_1     ide_reg_lba_1;
__sfr __at IDE_REG_LBA_2     ide_reg_lba_2;
__sfr __at IDE_REG_LBA_3     ide_reg_lba_3;
__sfr __at IDE_REG_SEC_COUNT ide_reg_sec_count;
__sfr __at IDE_REG_STATUS    ide_reg_status;

static void devide_read_data(void *buffer, uint8_t ioport) __naked
{
    buffer; ioport; /* silence compiler warning */
    __asm
            pop de              ; return address
            pop hl              ; buffer address
            pop bc              ; IO port number (in c)
            push bc             ; restore stack
            push hl
            push de

            ld b, #0            ; setup count
            inir                ; first 256 bytes
            inir                ; second 256 bytes
            ret
    __endasm;
}

static void devide_write_data(void *buffer, uint8_t ioport) __naked
{
    buffer; ioport; /* silence compiler warning */
    __asm
            pop de              ; return address
            pop hl              ; buffer address
            pop bc              ; IO port number (in c)
            push bc             ; restore stack
            push hl
            push de

            ld b, #0            ; setup count
            otir                ; first 256 bytes
            otir                ; second 256 bytes
            ret
    __endasm;
}

static bool devide_wait(uint8_t bits)
{
    uint8_t status;
    timer_t timeout;

    timeout = set_timer_ms(500);

    while(true){
        status = ide_reg_status;

        if((status & (IDE_STATUS_BUSY | IDE_STATUS_ERROR | bits)) == bits)
            return true;

        if((status & (IDE_STATUS_BUSY | IDE_STATUS_ERROR)) == IDE_STATUS_ERROR){
            kprintf("ide error, status=%x\n", status);
            return false;
        }

        if(timer_expired(timeout)){
            kprintf("ide timeout, status=%x\n", status);
            return false;
        }
    };
}

static bool devide_transfer_sector(uint8_t drive, uint32_t lba, void *buffer, bool read_notwrite)
{
    /* FIXME: only safe for LE, and only sensible for Z80/Z180 */
#if 0
    ide_reg_lba_3 = ((lba >> 24) & 0xF) | ((drive == 0) ? 0xE0 : 0xF0); // select drive, start loading LBA
    ide_reg_lba_2 = (lba >> 16);
    ide_reg_lba_1 = (lba >> 8);
    ide_reg_lba_0 = lba;
#else 
    /* sdcc sadly unable to figure this out for itself yet */
    uint8_t *p = (uint8_t *)&lba;
    ide_reg_lba_3 = (p[3] & 0x0F) | ((drive == 0) ? 0xE0 : 0xF0); // select drive, start loading LBA
    ide_reg_lba_2 = p[2];
    ide_reg_lba_1 = p[1];
    ide_reg_lba_0 = p[0];
#endif

    if(!devide_wait(IDE_STATUS_READY))
	return false;

    ide_reg_sec_count = 1;
    ide_reg_command = read_notwrite ? IDE_CMD_READ_SECTOR : IDE_CMD_WRITE_SECTOR;

    if(!devide_wait(IDE_STATUS_DATAREQUEST))
        return false;

    if(read_notwrite)
	devide_read_data(buffer, IDE_REG_DATA);
    else{
	devide_write_data(buffer, IDE_REG_DATA);
	if(!devide_wait(IDE_STATUS_READY))
	    return false;
	/* flush the cache after each write */
	if(drive_flags[drive] & FLAG_WRITE_CACHE){
	    ide_reg_command = IDE_CMD_FLUSH_CACHE;
	    if(!devide_wait(IDE_STATUS_READY))
		return false;
	}
    }

    return true;
}

/****************************************************************************/
/* Code below this point used only once, at startup, so we want it to live  */
/* in the DISCARD segment. sdcc only allows us to specify one segment for   */
/* each source file. This "solution" is a bit (well, very) hacky ...        */
/****************************************************************************/
static void DISCARDSEG(void) __naked { __asm .area _DISCARD __endasm; }

static void devide_delay(void)
{
    timer_t timeout;

    timeout = set_timer_ms(25);

    while(!timer_expired(timeout))
        platform_idle();
}

static void devide_init_drive(uint8_t drive)
{
    blkdev_t *blk;
    uint8_t *buffer, select;

    switch(drive){
	case 0: select = 0xE0; break;
	case 1: select = 0xF0; break;
        default: return;
    }

    drive_flags[drive] = 0;

    kprintf("IDE drive %d: ", drive);

    /* Reset depends upon the presence of alt control, which is optional */
#ifdef IDE_REG_CONTROL
    /* reset the drive */
    ide_reg_devhead = select;
    ide_reg_control = 0x06; /* assert reset, no interrupts */
    devide_delay();
    ide_reg_control = 0x02; /* release reset, no interruptst */
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
    ide_reg_command = IDE_CMD_IDENTIFY;

    /* allocate temporary sector buffer memory */
    buffer = (uint8_t *)tmpbuf();

    if(!devide_wait(IDE_STATUS_DATAREQUEST))
	goto failout;

    devide_read_data(buffer, IDE_REG_DATA);

    if(!(buffer[99] & 0x02)){
        kputs("LBA unsupported.\n");
        goto failout;
    }

    if( !(((uint16_t*)buffer)[82] == 0x0000 && ((uint16_t*)buffer)[83] == 0x0000) ||
         (((uint16_t*)buffer)[82] == 0xFFFF && ((uint16_t*)buffer)[83] == 0xFFFF) ){
	/* command set notification is supported */
	if(buffer[164] & 0x20){
	    /* write cache is supported */
	    drive_flags[drive] |= FLAG_WRITE_CACHE;
	}
    }

    blk = blkdev_alloc();
    if(!blk)
	goto failout;

    blk->transfer = devide_transfer_sector;
    blk->drive_number = drive;

    /* read out the drive's sector count */
    blk->drive_lba_count = *((uint32_t*)&buffer[120]);

    /* done with our temporary memory */
    brelse((bufptr)buffer);

    /* scan partitions */
    blkdev_scan(blk, SWAPSCAN);

    return;
failout:
    brelse((bufptr)buffer);
}

void devide_init(void)
{
    uint8_t d;

    for(d=0; d<DRIVE_COUNT; d++)
        devide_init_drive(d);
}
