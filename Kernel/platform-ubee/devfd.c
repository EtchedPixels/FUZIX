#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devfd.h>
#include <ubee.h>

/*
 *	Floppy controller logic for the Microbee systems with a 2793
 *	controller
 */

#define MAX_FD	4

#define OPDIR_NONE	0
#define OPDIR_READ	1
#define OPDIR_WRITE	2

#define FD_READ		0x80
#define FD_WRITE	0xA0

__sfr __at 0x58	fdc_devsel;

static uint8_t motorct;
static uint8_t fd_selected = 0xFF;
static uint8_t fd_tab[MAX_FD] = { 0xFF, 0xFF, 0xFF, 0xFF };

/*
 *	We only support normal block I/O for the moment. We do need to
 *	add swapping!
 */

/* Standard FDC */
static uint8_t selmap[4] = { 0x00, 0x01, 0x02, 0x03 };
#define FDC_SIDE1	0x04
#define FDC_DOUBLE	0x08
#define FDC_SINGLE	0x00

/* DreamDisc FDC */
static uint8_t selmap_dd[4] = { 0x01, 0x02, 0x04, 0x08 };
#define DDC_SIDE1	0x10
#define DDC_RATESEL	0x20
#define DDC_DOUBLE	0x00
#define DDC_SINGLE	0x40


static int fd_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
    blkno_t block;
    uint16_t dptr;
    uint16_t ct = 0;
    uint8_t tries;
    uint8_t err = 0;
    uint8_t *driveptr = fd_tab + minor;
    uint8_t cmd[7];

    minor &= 0x7F;

    if(rawflag)
        if (d_blkoff(9))
            return -1;

    if (fd_selected != minor) {
        uint8_t err;
        /* FIXME: We force DD for now. Side isn't handled here */
        err = fd_motor_on(selmap[minor & 0x7F]|FDC_DOUBLE);
        if (err)
            goto bad;
    }

    if (*driveptr == 0xFF)
        fd_reset(driveptr);

    dptr = (uint16_t)udata.u_dptr;
    block = udata.u_block;

    while(ct < udata.u_nblock) {
        cmd[0] = is_read ? FD_READ : FD_WRITE;
        /* Double sided assumed FIXME */
        cmd[1] = block / 20;
        /* floppy.s will sort the side out */
        cmd[2] = (block % 20) + 1;
        cmd[3] = is_read ? OPDIR_READ: OPDIR_WRITE;
        cmd[4] = dptr & 0xFF;
        cmd[5] = dptr >> 8;
        cmd[6] = rawflag;

        for (tries = 0; tries < 4 ; tries++) {
            err = fd_operation(cmd, driveptr);
            if (err == 0)
                break;
            if (tries > 1)
                fd_reset(driveptr);
        }
        if (tries == 4)
            goto bad;
        udata.u_block++;
        udata.u_dptr += 512;
        ct++;
    }
    return ct << BLKSHIFT;
bad:
    kprintf("fd%d: error %x\n", minor, err);
    udata.u_error = EIO;
    return -1;
}

int fd_open(uint8_t minor, uint16_t flag)
{
    uint8_t sel = (minor & 0x80) ? 1 : 0;
    flag;
    fdc_devsel = sel;
    if((disk_type[sel] != DISK_TYPE_FDC && disk_type[sel] != DISK_TYPE_FDC_D) ||
            minor >= MAX_FD) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int fd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    fdc_devsel = (minor & 0x80) ? 1: 0;
    return fd_transfer(minor, true, rawflag);
}

int fd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;rawflag;minor;
    fdc_devsel = (minor & 0x80) ? 1: 0;
    return fd_transfer(minor, false, rawflag);
}
