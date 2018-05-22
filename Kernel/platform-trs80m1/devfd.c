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

/* Extern as they live in common */
extern uint8_t fd_map, fd_tab[MAX_FD];
extern uint8_t fd_selected;
extern uint8_t fd_cmd[6];

/*
 *	We only support normal block I/O for the moment. We do need to
 *	add swapping!
 */
static uint8_t selmap[4] = { 0x01, 0x02, 0x04, 0x08 };

static int fd_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
    int ct = 0;
    int tries;
    uint8_t err = 0;
    uint8_t *driveptr = fd_tab + minor;

    if(rawflag == 2)
        goto bad2;

    /* FIXME: We force DD for now */
    err = fd_motor_on(selmap[minor]|0x80);
    if (err)
        goto bad;

    if (*driveptr == 0xFF)
        fd_reset(driveptr);

    fd_map = rawflag;
    if (rawflag && d_blkoff(BLKSHIFT))
            return -1;

    udata.u_nblock *= 2;

    fd_cmd[0] = is_read ? FD_READ : FD_WRITE;
    fd_cmd[1] = udata.u_block / 9;		/* 2 sectors per block */
    fd_cmd[2] = ((udata.u_block % 9) << 1) + 1;	/*eww.. */
    fd_cmd[3] = is_read ? OPDIR_READ: OPDIR_WRITE;
    fd_cmd[4] = ((uint16_t)udata.u_dptr) & 0xFF;
    fd_cmd[5] = ((uint16_t)udata.u_dptr) >> 8;

    while (ct < udata.u_nblock) {
        for (tries = 0; tries < 4 ; tries++) {
            err = fd_operation(driveptr);
            if (err == 0)
                break;
            if (tries > 1)
                fd_reset(driveptr);
        }
        /* FIXME: should we try the other half and then bale out ? */
        if (tries == 4)
            goto bad;
        fd_cmd[5]++;	/* Move on 256 bytes in the buffer */
        fd_cmd[2]++;	/* Next sector for 2nd block */
        ct++;
    }
    return udata.u_nblock << 8;
bad:
    kprintf("fd%d: error %x\n", minor, err);
bad2:
    udata.u_error = EIO;
    return -1;
}

int fd_open(uint8_t minor, uint16_t flag)
{
    flag;
    if(minor >= MAX_FD) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int fd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    return fd_transfer(minor, true, rawflag);
}

int fd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;rawflag;minor;
    return fd_transfer(minor, false, rawflag);
}
