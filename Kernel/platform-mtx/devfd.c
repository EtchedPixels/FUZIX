#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devfd.h>
#include <devtty.h>

/*
 *	TODO: Debug, low density is half the sectors/track,
 *	what to do about 80 v 40 track ?
 *
 */
/* Two drives but minors 2,3 are single density mode */
#define MAX_FD	4

#define OPDIR_NONE	0
#define OPDIR_READ	1
#define OPDIR_WRITE	2

#define FD_READ		0x88	/* 2797 needs 0x88, 1797 needs 0x80 */
#define FD_WRITE	0xA8	/* Likewise A8 v A0 */

static uint8_t motorct;
static uint8_t fd_selected = 0xFF;
static uint8_t fd_tab[MAX_FD] = { 0xFF, 0xFF };

void fd_motor_timer(void)
{
    if (motorct) {
        motorct--;
        if (motorct == 0) {
            fd_motor_off();
            fd_selected = 0xFF;
        }
    }
}

/*
 *	We only support normal block I/O not swap.
 */

static int fd_transfer(uint_fast8_t minor, bool is_read, uint_fast8_t rawflag)
{
    uint16_t nb = 0;
    int tries;
    uint8_t err = 0;
    uint8_t *driveptr = &fd_tab[minor & 1];
    irqflags_t irq;

    if(rawflag == 1 && d_blkoff(BLKSHIFT))
        return -1;

    udata.u_nblock *= 2;

    if (rawflag == 2)
        goto bad2;

    irq = di();
    if (fd_selected != minor) {
        uint8_t err = fd_motor_on(minor|(minor > 1 ? 0: 0x10));
        if (err)
            goto bad;
        motorct = 150;	/* 3 seconds */
    }
    irqrestore(irq);


//    kprintf("Issue command: %c drive %d block %d for %d\n", "wr"[is_read], minor, udata.u_block, udata.u_nblock);
    fd_cmd[0] = rawflag;
    fd_cmd[1] = is_read ? FD_READ : FD_WRITE;

    /* There are 16 256 byte sectors for DSDD. These are organised so that we
       switch head then step.

       Sectors 0-15 (our block 0-7)	Track 0, side 0
       Sectors 16-31 (our block 8-15)	Track 0, side 1

       etc */
    fd_cmd[2] = udata.u_block / 16;		/* Get the track we need */
    fd_cmd[3] = ((udata.u_block & 15) << 1);    /* 0 - 1 base is corrected in asm */
    fd_cmd[4] = is_read ? OPDIR_READ: OPDIR_WRITE;

    fd_data = (uint16_t)udata.u_dptr;

    while (udata.u_nblock--) {
        for (tries = 0; tries < 4 ; tries++) {
//            kprintf("Sector: %d Head: %d Track %d\n", (fd_cmd[3]&15)+1, fd_cmd[3]>>4, fd_cmd[2]);
            err = fd_operation(driveptr);
            if (err == 0)
                break;
            if (tries > 1)
                fd_reset(driveptr);
        }
        /* FIXME: should we try the other half and then bale out ? */
        if (tries == 3)
            goto bad;
        fd_data += 256;
        fd_cmd[3]++;	/* Next sector for next block */
        if (fd_cmd[3] == 32) {	/* Next track */
            fd_cmd[3] = 0;
            fd_cmd[2]++;
        }
        nb++;
    }
    return nb << (BLKSHIFT - 1);
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
    flag;
    return fd_transfer(minor, false, rawflag);
}
