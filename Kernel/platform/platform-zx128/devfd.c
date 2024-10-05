#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devfd.h>

#include "disciple.h"

#define MAX_FD	2

#define OPDIR_NONE	0
#define OPDIR_READ	1
#define OPDIR_WRITE	2

#define FD_READ		0x80
#define FD_WRITE	0xA2

/*
 *	TODO:
 *	- detect SD v DD and handle 256 byte sectors
 *	- detect SS v DS
 */

/* We are a bit rude with this as we just bash it without sharing */
__sfr __at 0x1F control;

/* Extern as they live in asm space */
extern uint8_t fd_tab[MAX_FD];
extern uint8_t fd_selected;
extern uint8_t fd_cmd[6];

static int fd_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
    blkno_t block;
    uint16_t dptr;
    int ct = 0;
    int tries;
    uint8_t err = 0;
    uint8_t *driveptr = fd_tab + minor;
    uint8_t nblock;
    uint8_t cval;
    uint8_t sec;

    if(rawflag == 2)
        goto bad2;

    /* 10 sectors per track, two sides, 512 bytes sector double density */
    /* FIXME fd_map support
    fd_map = rawflag;*/

    if (rawflag == 1)
        if (d_blkoff(BLKSHIFT))
            return -1;

    block = udata.u_block;
    dptr = (uint16_t)udata.u_dptr;

    fd_cmd[0] = is_read ? FD_READ : FD_WRITE;
    fd_cmd[1] = block / 20;
    sec = (block % 20) + 1;
    fd_cmd[3] = is_read ? OPDIR_READ: OPDIR_WRITE;
    fd_cmd[4] = dptr & 0xFF;
    fd_cmd[5] = dptr >> 8;

    cval = (minor ^ 1) | 4;

    if (*driveptr == 0xFF)
        fd_reset(driveptr);


    while (ct < udata.u_nblock) {
        if (sec > 10) {
            control = cval | 2;
            fd_cmd[2] = sec - 10;
        } else {
            control = cval;
            fd_cmd[2] = sec;
        }
        if (!is_read && fd_cmd[1] > 64)
            fd_cmd[0] |= 4;	/* Write precompensation on */

        for (tries = 0; tries < 4 ; tries++) {
            err = fd_operation(driveptr);
            if (err == 0)
                break;
            if (tries > 1)
                fd_reset(driveptr);
        }
        /* FIXME: need to do SD v DD detection */
        if (tries == 4)
            goto bad;
        fd_cmd[5]+= 2;	/* Move on 512 bytes in the buffer */
        sec++;
        /* Step a track */
        if (sec > 20) {
            sec = 1;
            fd_cmd[1]++;
        }
        ct++;
    }
    return 1;
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
    flag;
    return fd_transfer(minor, false, rawflag);
}
