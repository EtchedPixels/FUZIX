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
uint8_t block_units[8];
uint8_t readonly;

#define dos_cmd	((uint8_t *)0x42)

static int hd_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
    uint16_t dptr, nb;
    uint8_t err;
    uint8_t slot = minor >> 5;
    uint8_t slotbit = 1 << slot;

    /* FIXME: swap support */
    if(rawflag == 1 && d_blkoff(9))
        return -1;

    hd_map = rawflag;

    dptr = (uint16_t)udata.u_dptr;
    nb = udata.u_nblock;
        
    while (udata.u_nblock--) {
        /* Fill in the protocol convertor information */
        /* Note hd_map will be fun to handle */
        if (pascal_slot & slotbit) {
            pascal_cmd[0] = (minor & 0x1F) + 1;
            pascal_cmd[1] = dptr;
            pascal_cmd[2] = dptr >> 8;
            pascal_cmd[3] = udata.u_block;
            pascal_cmd[4] = udata.u_block >> 8;
            pascal_cmd[5] = 0;	/* If we do big disks this will be 16-23 */
            if (rawflag)
                pascal_cmd[5] = 0x80;	/* Should select alt bank */
            err = pascal_op((is_read ? 0x0300 : 0x0400) | slot);
        } else {
            /* DOS mode only allows 2 devices/slot */
            /* TODO: No alt bank access */
            dos_cmd[0] = is_read ? 1 : 2;
            /* FIXME: check these are correct shifts */
            dos_cmd[1] = ((minor & 0xE0) >> 2);
            if (minor & 1)
                dos_cmd[1] |= 0x80;
            dos_cmd[2] = dptr;
            dos_cmd[3] = dptr >> 8;
            dos_cmd[4] = udata.u_block;
            dos_cmd[5] = udata.u_block >> 8;
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

static uint8_t statusdata[8];

static uint8_t pascal_status(uint8_t slot, uint8_t unit)
{
    pascal_cmd[0] = 3;
    pascal_cmd[1] = unit;
    pascal_cmd[2] = (uint8_t)statusdata;
    pascal_cmd[3] = ((uint16_t)statusdata) >> 8;
    pascal_cmd[4] = 0;
    return pascal_op(slot);
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
        if (pascal_status(slot, unit + 1) || (statusdata[0] & 0xA0) != 0xA0) {
            udata.u_error = ENXIO;
            return -1;
        }
        /* It's readable and block - good signs */
        if ((statusdata[0] & 4) || (O_ACCMODE(flag) && !(statusdata[0] & 0x40))) {
            udata.u_error = EROFS;
            return -1;
        }
        /* Offline / No Media */
        if (!(statusdata[0] & 0x10)) {
            udata.u_error = EIO;
            return -1;
        }
    } else {
        /* Driver ROM reports it is an R/O interface */
        if ((readonly & (1 << slot)) && O_ACCMODE(flag)) {
            udata.u_error = EROFS;
            return -1;
        }
        /* Status check */
        dos_cmd[0] = 0;
        dos_cmd[1] = (slot << 3);
        if (minor)
            dos_cmd[1] |= 0x80;
        /* DOS status op - check if we are ready */
        if (dos_op(slot)) {
            udata.u_error = EIO;
            return -1;
        }
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

/*
 * Initialize a discovered hard disk interface. Needs to move into discard
 * space eventually. We query the interface to see if it makes sense and
 * record any read only or other information. We know how to use both the
 * pascal interface (preferred) and the ProDOS one.
 */

void hd_install(uint8_t slot)
{
    register uint8_t *p = (uint8_t *)0xC000 + (((uint16_t)slot) << 8);
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
    }
    /* And at this point we ought to scan it for a swap signature */
}
