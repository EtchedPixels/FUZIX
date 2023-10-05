#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devfd.h>

#define MAX_FD	4

#define OPDIR_NONE	0
#define OPDIR_READ	1
#define OPDIR_WRITE	2

#define FD_READ		0x80	/* 2797 needs 0x88, 1797 needs 0x80 */
#define FD_WRITE	0xA0	/* Likewise A8 v A0 */

static uint8_t motorct;
static uint8_t fd_selected = 0xFF;
static uint8_t fd_tab[MAX_FD] = { 0xFF, 0xFF, 0xFF, 0xFF };

/*
 *	We only support normal block I/O for the moment. We do need to
 *	add swapping!
 */

static uint8_t selmap[4] = { 0x01, 0x02, 0x04, 0x08 };

static int fd_transfer(uint_fast8_t minor, bool is_read, uint_fast8_t rawflag)
{
    int ct = 0;
    int tries;
    uint8_t err = 0;
    uint8_t *driveptr = fd_tab + minor;
    uint8_t cmd[6];

    if(rawflag == 2)
        goto bad2;

    if (fd_selected != minor) {
        uint8_t err;
        /* FIXME: We force DD for now */
        err = fd_motor_on(selmap[minor]|0x80);
        if (err)
            goto bad;
    }

    if (*driveptr == 0xFF)
        fd_reset(driveptr);

    if (rawflag && d_blkoff(BLKSHIFT))
            return -1;

    cmd[0] = is_read ? FD_READ : FD_WRITE;
    cmd[1] = udata.u_block / 9;		/* 2 sectors per block */
    cmd[2] = ((udata.u_block % 9) << 1) + 1;	/*eww.. */
    cmd[3] = is_read ? OPDIR_READ: OPDIR_WRITE;
    cmd[4] = ((uint16_t)udata.u_dptr) & 0xFF;
    cmd[5] = ((uint16_t)udata.u_dptr) >> 8;

    while (ct < 2) {
        for (tries = 0; tries < 4 ; tries++) {
            err = fd_operation(cmd, driveptr);
            if (err == 0)
                break;
            if (tries > 1)
                fd_reset(driveptr);
        }
        /* FIXME: should we try the other half and then bale out ? */
        if (tries == 4)
            goto bad;
        cmd[5]++;	/* Move on 256 bytes in the buffer */
        cmd[2]++;	/* Next sector for 2nd block */
        ct++;
    }
    return udata.u_nblock << 8;
bad:
    kprintf("fd%d: error %x\n", minor, err);
bad2:
    udata.u_error = EIO;
    return -1;
}

int fd_open(uint_fast8_t minor, uint16_t flag)
{
    flag;
    if(minor >= MAX_FD) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int fd_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
    flag;
    return fd_transfer(minor, true, rawflag);
}

int fd_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
    flag;rawflag;minor;
//    return 0;
    return fd_transfer(minor, false, rawflag);
}
