/*-----------------------------------------------------------------------*/
/* IDE interface driver                                                  */
/* 2014-11-02 Will Sowerbutts (unreleased UZI-mark4)                     */
/* 2014-12-22 WRS ported to Fuzix                                        */
/* 2014-12-25 WRS updated to also support P112 GIDE                      */
/* 2015-01-04 WRS updated to new blkdev API                              */
/* 2015-01-25 WRS updated to newer blkdev API                            */
/*-----------------------------------------------------------------------*/

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <timer.h>
#include <devide.h>
#include <blkdev.h>

#define DRIVE_COUNT 2           /* at most 2 drives without adjusting DRIVE_NR_MASK */

/* we use the bits in the driver_data field of blkdev_t as follows: */
#define DRIVE_NR_MASK    0x01   /* low bit used to select the drive number -- extend if more required */
#define FLAG_CACHE_DIRTY 0x40
#define FLAG_WRITE_CACHE 0x80

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

static void devide_read_data(void);
static void devide_write_data(void);

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

static uint8_t devide_transfer_sector(void)
{
    uint8_t drive;
#if defined(__SDCC_z80) || defined(__SDCC_z180) || defined(__SDCC_gbz80) || defined(__SDCC_r2k) || defined(__SDCC_r3k)
    uint8_t *p;
#endif

    drive = blk_op.blkdev->driver_data & DRIVE_NR_MASK;

#if defined(__SDCC_z80) || defined(__SDCC_z180) || defined(__SDCC_gbz80) || defined(__SDCC_r2k) || defined(__SDCC_r3k)
    /* sdcc sadly unable to figure this out for itself yet */
    p = ((uint8_t *)&blk_op.lba)+3;
    ide_reg_lba_3 = (*(p--) & 0x0F) | ((drive == 0) ? 0xE0 : 0xF0); // select drive, start loading LBA
    ide_reg_lba_2 = *(p--);
    ide_reg_lba_1 = *(p--);
    ide_reg_lba_0 = *p;
#else
    ide_reg_lba_3 = ((blk_op.lba >> 24) & 0xF) | ((drive == 0) ? 0xE0 : 0xF0); // select drive, start loading LBA
    ide_reg_lba_2 = (blk_op.lba >> 16);
    ide_reg_lba_1 = (blk_op.lba >> 8);
    ide_reg_lba_0 = blk_op.lba;
#endif

    if(!devide_wait(IDE_STATUS_READY))
	return 0;

    ide_reg_sec_count = 1;
    ide_reg_command = blk_op.is_read ? IDE_CMD_READ_SECTOR : IDE_CMD_WRITE_SECTOR;

    if(!devide_wait(IDE_STATUS_DATAREQUEST))
        return 0;

    if(blk_op.is_read)
	devide_read_data();
    else{
	devide_write_data();
	if(!devide_wait(IDE_STATUS_READY))
	    return 0;
	blk_op.blkdev->driver_data |= FLAG_CACHE_DIRTY;
    }

    return 1;
}

static int devide_flush_cache(void)
{
    uint8_t drive;

    drive = blk_op.blkdev->driver_data & DRIVE_NR_MASK;

    /* check drive has a cache and was written to since the last flush */
    if(blk_op.blkdev->driver_data & (FLAG_WRITE_CACHE | FLAG_CACHE_DIRTY)
		                 == (FLAG_WRITE_CACHE | FLAG_CACHE_DIRTY)){
	ide_reg_lba_3 = ((drive == 0) ? 0xE0 : 0xF0); // select drive

	if(!devide_wait(IDE_STATUS_READY)){
	    udata.u_error = EIO;
	    return -1;
	}

	ide_reg_command = IDE_CMD_FLUSH_CACHE;

	if(!devide_wait(IDE_STATUS_READY)){
	    udata.u_error = EIO;
	    return -1;
	}

        /* drive cache is now clean */
	blk_op.blkdev->driver_data &= ~FLAG_CACHE_DIRTY;
    }

    return 0;
}

/****************************************************************************/
/* The innermost part of the transfer routines has to live in common memory */
/* since it must be able to bank switch to the user memory bank.            */
/****************************************************************************/
COMMON_MEMORY

static void devide_read_data(void) __naked
{
    __asm
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
            ld b, #0                                ; setup count
            ld c, #IDE_REG_DATA                     ; setup port number
            or a                                    ; test is_user
            jr z, goread                            ; just start the transfer if kernel memory
            call map_process_always                 ; else map user memory first
goread:     inir                                    ; transfer first 256 bytes
            inir                                    ; transfer second 256 bytes
            or a                                    ; test is_user
            ret z                                   ; done if kernel memory transfer
            jp map_kernel                           ; else map kernel then return
    __endasm;
}

static void devide_write_data(void) __naked
{
    __asm
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
            ld b, #0                                ; setup count
            ld c, #IDE_REG_DATA                     ; setup port number
            or a                                    ; test is_user
            jr z, gowrite                           ; just start the transfer if kernel memory
            call map_process_always                 ; else map user memory first
gowrite:    otir                                    ; transfer first 256 bytes
            otir                                    ; transfer second 256 bytes
            or a                                    ; test is_user
            ret z                                   ; done if kernel memory transfer
            jp map_kernel                           ; else map kernel then return
    __endasm;
}

/****************************************************************************/
/* Code below this point used only once, at startup, so we want it to live  */
/* in the DISCARD segment. sdcc only allows us to specify one segment for   */
/* each source file. This "solution" is a bit (well, very) hacky ...        */
/****************************************************************************/
DISCARDABLE

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
}

void devide_init(void)
{
    uint8_t d;

    for(d=0; d<DRIVE_COUNT; d++)
        devide_init_drive(d);
}
