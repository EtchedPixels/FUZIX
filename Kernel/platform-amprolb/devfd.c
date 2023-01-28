
/*
 *	High level floppy drive for the Ampro Littleboard
 *
 *	TODO: disk ioctl interface to allow multiple media types
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include "floppy.h"

#define MAX_FD	4

static uint8_t fd_track[MAX_FD];

/* Set the drive select, density and head */
static void fd_setup(uint8_t minor, uint8_t head)
{
    head <<= 4;
    head |= 0x20;		/* DD */
    head |= (1 << minor);
    set_latch(0xC000 + head);	/* Preserve clock and bank bits */
}

/* For now this assumes DD 10 sector per track with the usual weird
   sector offset. The native formats are apparently
   
   Type  T   S    H  SZ   OFFS
   SSDD	 40  10   1  512  1		200K
   DSDD  40  10   2  512  17		400K
   DSDD  80  10   2  512  17?		800K
       (maybe 5 x 1024 which if so we can't handle nicely)

   and we should also add PC
   SSDD  40   9   1  512  1		360K PC
   DSDD  40   9   2  512  1		720K PC (rarer)
*/
   
static int fd_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
    unsigned ct = 0;
    uint8_t err = 0xFF;
    uint8_t sec, head, track, retries, last_track;
    irqflags_t irq;
    
    if (rawflag == 2)
        return -1;
    if (rawflag)
        d_blkoff(9);

    last_track = fd_track[minor];

    wd_map = rawflag;

    sec = udata.u_block % 20;
    track = udata.u_block / 20;
    if (sec >= 10) {
        sec -= 10;
        head = 1;
    } else
        head = 0;

    fd_setup(minor, head);

    /* Load the track register from the saved position if known. If
       not then we will seek anyway */
    last_track = fd_track[minor];
    if (last_track != 0xFF)
        wd_setup(last_track << 8);

    while(ct < udata.u_nblock) {
        err = 0xFF;
        retries = 0;
        while (err && retries++ < 5) {
            if (retries > 3) {
                kprintf("fd%d: disk error, retrying.\n", minor);
                if (wd_restore()) {
                    last_track = 0xFF;
                    continue;
                }
                last_track = 0;
            }
            /* Position the head. If the old value was FF it means
               we and the drive don't know */
            if (last_track == 0xFF) {
                if (wd_restore())
                    continue;
                last_track = 0;
            }
            if (last_track != track) {
                if (wd_seek(track)) {
                    last_track = 0xFF;
                    continue;
                }
                last_track = track;
            }
            wd_setup((track << 8) | sec + 17);
            /* We need the interrupts off for real during floppy transfers */
            irq = __hard_di();
            if (is_read)
                err = wd_read(udata.u_dptr);
            else
                err = wd_write(udata.u_dptr);
            __hard_irqrestore(irq);
        };
        if (err)
            break;
        udata.u_dptr += 512;
        sec++;
        if (sec == 10) {
            sec = 0;
            head ^= 1;
            fd_setup(minor, head);
            if (head == 0)
                track++;
        }
        ct++;
    }
    /* Operation complete */
    fd_track[minor] = last_track;
    if (err) {
        kprintf("fd%d: disk error %2x.\n", minor, err);
        /* Need to look at this in general for disk I/O */
        if (ct == 0) {
            udata.u_error = EIO;
            return -1;
        }
    }
    return ct << BLKSHIFT;
}        

int fd_open(uint8_t minor, uint16_t flag)
{
    uint_fast8_t err;

    if(minor >= MAX_FD) {
        udata.u_error = ENODEV;
        return -1;
    }

    fd_setup(minor, 0);
    err = wd_restore();

    if (err) {
        fd_track[minor] = 0xFF;
        if (!(flag & O_NDELAY)) {
            udata.u_error = EIO;
            return -1;
        }
    } else
        fd_track[minor] = 0;
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
