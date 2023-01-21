#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devfdc.h>

/*
 *	16 sectors / track	128 byte SD 256 byte DD
 *	40 or 80 tracks
 *	Under the base OS the second side is a different disk
 *
 *	Assume DD for now. Density bit is in 6058 0 = double
 *
 *	TODO: sort out drive layout. We can have mixed SD and DD
 *	drives so it could be that 0 is a real drive but 1/2 is a 640 ?
 */

#define MAX_FD	4

uint8_t fd_map;

static int fd_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
    uint16_t block;
    uint16_t ct = 0;
    uint8_t err;

    /* You can't swap to floppy */
    if(rawflag == 2)
        goto bad2;

    fd_map = rawflag;
    if (rawflag) {
        if (d_blkoff(8))
            return -1;
        block = udata.u_block;
    } else {
        udata.u_nblock *= 2;
        block = udata.u_block;
        block *= 2;
    }

    fdbios_drive = minor << 1;
    fdbios_op = is_read ? FDOP_READ : FDOP_WRITE;

    /* Floppies are 320K or 640K. 640K media acts as if it were two drives */
    while (ct < udata.u_nblock) {
        if (block > 1279) {
            fdbios_drive |= 1;		/* Switch to second head */
            block -= 1280;		/* Back to start of second "disk" */
        }
        fdbios_track = block >> 4;		/* 604A track: 16bit */
        fdbios_sector = (block & 0x0F) + 1;	/* 16 spt */
        fdbios_addr = udata.u_dptr;
        if (err = fdbios_flop())	/* Handle fd bios interface and mapping */
            goto bad;
        udata.u_dptr += 256;
        udata.u_block++;
        ct++;
    }
    return ct << 8;
bad:
    kprintf("fd%d: error %x\n", minor, err);
bad2:
    udata.u_error = EIO;
    return -1;
}

int fd_open(uint8_t minor, uint16_t flag)
{
    if(minor >= MAX_FD) {
        udata.u_error = ENODEV;
        return -1;
    }
    /* No floppy: TODO check properly and check type 'D' v 'S' v 'Q' etc */
    if (fdbios_floppy == 0xFF)
        return 0;
    return -1;
}

int fd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    return fd_transfer(minor, true, rawflag);
}

int fd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    return fd_transfer(minor, false, rawflag);
}

int fd_ioctl(uint8_t minor, uarg_t request, char *buffer)
{
    return -1;
}
