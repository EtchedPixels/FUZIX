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
 *	PASCAL or ProDOS entry points ?
 *	if it reports removable ensure we flush buffers on close
 *	if it reports formatting add an ioctl
 *	if it doesn't report interruptible ensure the asm cli's it
 *
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devhd.h>

extern uint8_t hd_map;
uint8_t rw_cmd[7];
uint8_t block_units[8];
uint8_t readonly;

static int hd_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
    uint16_t dptr, nb;
    uint8_t err;

    /* FIXME: swap support */
    if(rawflag == 1 && d_blkoff(9))
        return -1;

    hd_map = rawflag;

    dptr = (uint16_t)udata.u_dptr;
    nb = udata.u_nblock;
        
    while (udata.u_nblock--) {
        /* Fill in the protocol convertor information */
        /* Note hd_map will be fun to handle */
#ifdef CONFIG_PASCAL        
        rw_cmd[0] = is_read ? 0x03 : 0x04;
        rw_cmd[1] = minor & 0x1F;
        rw_cmd[2] = dptr;
        rw_cmd[3] = dptr >> 8;
        rw_cmd[4] = udata.u_block;
        rw_cmd[5] = udata.u_block >> 8;
        rw_cmd[6] = 0;	/* If we do big disks this will be 16-23 */
#else
        /* DOS mode only allows 2 devices/slot */
        rw_cmd[0] = is_read ? 1 : 2;
        /* FIXME: check these are correct shifts */
        rw_cmd[1] = ((minor & 0xE0) >> 2);
        rw_cmd[1] |= (minor & 3) << 6;
        rw_cmd[2] = dptr;
        rw_cmd[3] = dptr >> 8;
        rw_cmd[4] = udata.u_block;
        rw_cmd[5] = udata.u_block >> 8;
#endif        

        err = block_rw();

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
    used(flag);

#ifndef CONFIG_PASCAL
    /* FIXME: may need a core tweak to match */
    if ((readonly & (1 << slot)) && O_ACCMODE(flag)) {
        udata.u_error = EROFS;
        return -1;
    }
#endif
    /* Non block device slots have 0 here */
    if ((minor & 0x1F) >= block_units[slot]) {
        udata.u_error = ENXIO;
        return -1;
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

    if (!(p[254] & 4))
        readonly |= (1 << slot);
    block_units[slot] = (p[254] >> 4) & 3 + 1;   
    /* And at this point we ought to scan it for a swap signature */
}
