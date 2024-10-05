#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <tinydisk.h>
#include <mbc2.h>

static uint8_t last_drive = 0xFF;

/*
 *	Simple virtual drives (no raw SD access alas)
 */

int vd_xfer(uint_fast8_t drive, bool is_read, uint32_t lba, uint8_t *dptr)
{
    irqflags_t irq;
    uint16_t track;
    uint8_t sector;
    uint8_t c;

    irq = di();

    if (drive != last_drive) {
        out(opcode, OP_SET_DISK);
        out(opwrite, drive);
        last_drive = drive;
        out(opcode, OP_GET_ERROR);
        c = in(opread);
        if (c) {
            kprintf("hd: drive %d select failed %d.\n", drive, c);
            last_drive = 255;
            irqrestore(irq);
            return 0;
        }
    }

    track = lba >> 5;
    sector = (uint16_t)lba & 0x1F;

    out(opcode , OP_SET_TRACK);
    out(opwrite, track);
    out(opwrite, track >> 8);
    out(opcode, OP_SET_SECTOR);
    out(opwrite, sector);

    if (is_read)
        vd_read(dptr);
    else
        vd_write(dptr);

    out(opcode, OP_GET_ERROR);
    c = in(opread);
    irqrestore(irq);

    if (c) {
        kprintf("hd: drive %d error %d.\n", drive, c);
        return 0;
    }
    return 1;
}

int vd_flush(void)
{
    return 0;
}

void vd_init_drive(uint_fast8_t drive)
{
    irqflags_t irq = di();
    out(opcode, OP_SET_DISK);
    out(opwrite,  drive);
    out(opcode, OP_GET_ERROR);
    if (in(opread) == 0)
        td_register(drive, vd_xfer, td_ioctl_none, 1);
    last_drive = 255;
    irqrestore(irq);
}

void vd_init(void)
{
    uint_fast8_t d;
    for (d = 0; d < VD_DRIVE_COUNT; d++)
        vd_init_drive(d);
}        
