/*-----------------------------------------------------------------------*/
/* IDE interface driver                                                  */
/* 2014-11-02 Will Sowerbutts (unreleased UZI-mark4)                     */
/* 2014-12-22 WRS ported to Fuzix                                        */
/* 2014-12-25 WRS updated to also support P112 GIDE                      */
/*-----------------------------------------------------------------------*/

/* Minor numbers

   The kernel identifies our storage devices with an 8-bit minor number.

   The top two bits of the minor identify the drive, allowing a maximum of four
   drives, the current hardware supports only two but future controllers may
   support more.
   
   The bottom six bits of the minor identify the slice number. Slice 0
   addresses "the whole drive", with no translation. Due to limitations within
   the kernel only the first 32MB can currently be accessed through slice 0.

   Slice 0 is intended to be used primarily for writing a partition table to
   the drive, although it can also be used to store a single Fuzix filesystem
   on an unpartitioned disk.
   
   Slices 1+ are stored in a partition on the drive. The partition is stored in
   a PC-style MBR partition table and must be a primary partition of type 0x5A.
   Only the first suitable partition is found. Multiple filesystems are stored
   in this partition by dividing it into 32MB slices.

   To create the required device nodes, use:

       mknod /dev/hda 60660 0
       mknod /dev/hdb 60660 64
       mknod /dev/hdc 60660 128
       mknod /dev/hdd 60660 192

       mknod /dev/hda1 60660 1
       mknod /dev/hda2 60660 2
       mknod /dev/hda3 60660 3
       ...
       mknod /dev/hda63 60660 63

       mknod /dev/hdb1 60660 65
       mknod /dev/hdb2 60660 66
       mknod /dev/hdb3 60660 67
       ...
       mknod /dev/hdb63 60660 127

       mknod /dev/hdc1 60660 129
       mknod /dev/hdc2 60660 130
       mknod /dev/hdc3 60660 131
       ...
       mknod /dev/hdc63 60660 191

       mknod /dev/hdd1 60660 193
       mknod /dev/hdd2 60660 194
       mknod /dev/hdd3 60660 195
       ...
       mknod /dev/hdd63 60660 255
*/

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <timer.h>
#include <devide.h>
#include <mbr.h>

#define DRIVE_COUNT 2   /* range 1 -- 4 */
#define MAX_SLICES 63

static uint8_t  ide_drives_present; /* bitmap */
static uint32_t ide_partition_start[DRIVE_COUNT];
static uint8_t  ide_slice_count[DRIVE_COUNT];

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

static void devide_delay(void)
{
    timer_t timeout;

    timeout = set_timer_ms(25);

    while(!timer_expired(timeout))
        platform_idle();
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

static bool devide_read_sector(uint8_t *buffer)
{
    if(!devide_wait(IDE_STATUS_DATAREQUEST))
        return false;

    devide_read_data(buffer, IDE_REG_DATA);

    return true;
}

static bool devide_write_sector(uint8_t *buffer)
{
    if(!devide_wait(IDE_STATUS_DATAREQUEST))
        return false;

    devide_write_data(buffer, IDE_REG_DATA);

    if(!devide_wait(IDE_STATUS_READY))
        return false;

    return true;
}

static int devide_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
    uint8_t *target, *p;
    uint32_t lba;
    uint8_t drive;

    drive = minor >> 6;
    minor = minor & 0x3F;

    if(rawflag == 0) {
        target = udata.u_buf->bf_data;
        lba = udata.u_buf->bf_blk;
    }else
        goto xferfail;

    /* minor 0 is the whole disk and requires no translation */
    if(minor > 0){
        /* minor 1+ are slices located within the partition */
        lba += (ide_partition_start[drive]);
        lba += ((uint32_t)(minor-1) << SLICE_SIZE_LOG2_SECTORS);
    }

#if 0
    ide_reg_lba_3 = ((lba >> 24) & 0xF) | ((drive == 0) ? 0xE0 : 0xF0); // select drive, start loading LBA
    ide_reg_lba_2 = (lba >> 16);
    ide_reg_lba_1 = (lba >> 8);
    ide_reg_lba_0 = lba;
#else
    /* sdcc sadly unable to figure this out for itself yet */
    p = (uint8_t *)&lba;
    ide_reg_lba_3 = (p[3] & 0x0F) | ((drive == 0) ? 0xE0 : 0xF0); // select drive, start loading LBA
    ide_reg_lba_2 = p[2];
    ide_reg_lba_1 = p[1];
    ide_reg_lba_0 = p[0];
#endif
    ide_reg_sec_count = 1;

    if(is_read){
        ide_reg_command = IDE_CMD_READ_SECTOR;
        if(!devide_read_sector(target))
            goto xferfail;
    }else{
        ide_reg_command = IDE_CMD_WRITE_SECTOR;
        if(!devide_write_sector(target))
            goto xferfail;
    }

    return 1;
xferfail:
    udata.u_error = EIO;
    return -1;
}

int devide_open(uint8_t minor, uint16_t flags)
{
    uint8_t drive;
    flags; /* not used */

    drive = minor >> 6;
    minor = minor & 0x3F;

    if(ide_drives_present & (1 << drive) && (minor == 0 || minor < ide_slice_count[drive]))
        return 0;

    udata.u_error = ENODEV;
    return -1;
}

int devide_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag; /* not used */
    return devide_transfer(minor, true, rawflag);
}

int devide_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag; /* not used */
    return devide_transfer(minor, false, rawflag);
}

void devide_init_drive(uint8_t drive)
{
    uint8_t *buffer, select;

    switch(drive){
        case 0: select = 0xE0; break;
        case 1: select = 0xF0; break;
        default: return;
    }

    kprintf("hd%c: ", 'a' + drive);
    ide_partition_start[drive] = 0;
    ide_slice_count[drive] = 0;

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
    ide_reg_command = IDE_CMD_IDENTIFY;

    /* allocate temporary sector buffer memory */
    buffer = (uint8_t *)tmpbuf();

    if(!devide_read_sector(buffer))
        goto failout;

    if(!(buffer[99] & 0x02)){
        kputs("LBA unsupported.\n");
        goto failout;
    }

    /* read first sector (looking for the partition table) */
    if(!devide_wait(IDE_STATUS_READY))
        goto failout;

    ide_reg_devhead = select;
    ide_reg_lba_2 = 0;
    ide_reg_lba_1 = 0;
    ide_reg_lba_0 = 0;
    ide_reg_sec_count = 1;
    ide_reg_command = IDE_CMD_READ_SECTOR;

    if(!devide_read_sector(buffer))
        goto failout;

    /* if we get this far the drive is apparently present and functioning */
    ide_drives_present |= (1 << drive);

    parse_partition_table(buffer, &ide_partition_start[drive], &ide_slice_count[drive], MAX_SLICES);

failout:
    brelse((bufptr)buffer);
}

void devide_init(void)
{
    uint8_t d;

    ide_drives_present = 0;

    for(d=0; d<DRIVE_COUNT; d++)
        devide_init_drive(d);
}
