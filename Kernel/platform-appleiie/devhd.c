/*
 *	There are essentially two kinds of disks on the Apple IIe/c
 *	- The ancient 140K floppies which need an OS level driver
 *	- SmartPort type devices with a common (and quite nice) firmware
 *	  interface.
 *
 *	This driver handles SmartPort devices. It assumes no more than 32
 *	devices per slot and no more than 8 slots. Device partitioning
 *	on the Apple IIe can be a strange mix of hardware and software.
 *	Usually the firmware presents the real device as multiple actual
 *	smartport devices.
 *
 *	TODO:
 *	if it reports removable ensure we flush buffers on close
 *	if it reports formatting add an ioctl
 *	if it doesn't report interruptible ensure the asm cli's it
 *
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devhd.h>
#include <apple.h>

extern uint8_t hd_map;
uint8_t rw_cmd[7];
uint8_t block_units[8];
uint8_t readonly;

static int hd_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
    uint16_t dptr, nb;
    uint8_t err;
    uint8_t slotbit = 1 << (minor >> 5);

    /* FIXME: swap support */
    if(rawflag == 1 && d_blkoff(9))
        return -1;

    hd_map = rawflag;

    dptr = (uint16_t)udata.u_dptr;
    nb = udata.u_nblock;
        
    while (udata.u_nblock--) {
        /* Fill in the protocol convertor information */
        /* Note hd_map will be fun to handle */

        rw_cmd[2] = dptr;
        rw_cmd[3] = dptr >> 8;
        rw_cmd[4] = udata.u_block;
        rw_cmd[5] = udata.u_block >> 8;
        rw_cmd[6] = 0;	/* If we do big disks this will be 16-23 */

        if (pascal_slot & slotbit) {
            rw_cmd[0] = is_read ? 0x03 : 0x04;
            rw_cmd[1] = minor & 0x1F;
            if (rawflag)
                rw_cmd[6] = 0x80;	/* Should select alt bank */
            err = block_rw_pascal();
        } else {
            /* DOS mode only allows 2 devices/slot */
            /* TODO: No alt bank access */
            rw_cmd[0] = is_read ? 1 : 2;
            /* FIXME: check these are correct shifts */
            rw_cmd[1] = ((minor & 0xE0) >> 2);
            if (minor & 1)
                rw_cmd[1] |= 0x80;
            rw_cmd[1] |= (minor & 1) << 6;
            err = block_rw_prodos();
        }

        if (err) {
            kprintf("hd%d: disk error %x\n", err);
            udata.u_error = EIO;
            return -1;
        }

        udata.u_block++;
        dptr += 512;

    }
    return nb;
}

int hd_open(uint8_t minor, uint16_t flag)
{
    uint8_t slot = minor >> 5;
    uint8_t unit = minor & 31;
    used(flag);

    /* Non block device slots have 0 here */
    if (unit >= block_units[slot]) {
        udata.u_error = ENXIO;
        return -1;
    }
    if (pascal_slot & (1 << slot)) {
        if (pascal_status(slot, unit) || (statusdata[0] & 0xA0) != 0xA0) {
            udata.u_error = ENXIO;
            return -1;
        }
        /* It's readable and block - good signs */
        if ((statusdata[0] & 4) || (O_ACCMODE(flag) && !(statusdata[0] & 0x40))) {
            udata.u_error = EROFS;
            return -1;
        }
        if (!(statusdata[0] & 0x10)) {
            /* FIXME: better errno code */
            udata.u_error = ENXIO;
            return -1;
        }
    } else {
        if (( readonly & (1 << slot)) && O_ACCMODE(flag)) {
            udata.u_error = EROFS;
            return -1;
        }
        /* FIXME: ProDOS status wants doing here for media check */
    }
    return 0;
}

int hd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    used(flag);
    return hd_transfer(minor, true, rawflag);
}

int hd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    used(flag);
    return hd_transfer(minor, false, rawflag);
}

/* This assumes a ProDOS type structure is present and we are doing
   ProDOS things */
void hd_install(uint8_t slot)
{
    uint8_t *p = (uint8_t *)0xC000 + (((uint16_t)slot) << 8);
    uint8_t err;

    if (pascal_slot & (1 << slot)) {
        err = pascal_status(slot, 0);
        if (err) {
            kprintf("Slot %d device install failed: stat %d\n", slot, err);
            return;
        }
        block_units[slot] = statusdata[0];
    } else {
        if (!(p[254] & 4))
            readonly |= (1 << slot);
        block_units[slot] = (p[254] >> 4) & 3 + 1;
        /* And at this point we ought to scan it for a swap signature */
    }
}
