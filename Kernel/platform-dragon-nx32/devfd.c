#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include "devfd.h"

#define MAX_FD	4

#define OPDIR_NONE	0
#define OPDIR_READ	1
#define OPDIR_WRITE	2

#define FD_READ		0x88	/* 2797 needs 0x88, 1797 needs 0x80 */
#define FD_WRITE	0xA8	/* Likewise A8 v A0 */

static uint8_t motorct;
static uint8_t fd_selected = 0xFF;
extern uint8_t fd_tab[];

static uint8_t motor_timeout = 0;

static inline void fd_motor_busy(void)
{
    motorct++;
    motor_timeout = 240;
}

static inline void fd_motor_idle(void)
{
    motorct--;
}

void fd_timer_tick(void)
{
    if (!motorct && motor_timeout) {
        if (motor_timeout-- == 1) {
            fd_selected = 0xff;
            fd_motor_off();
        }
    }
}

/*
 *	We only support normal block I/O because otherwise we'd need
 *	bounce buffers - which would make it just as pointless!
 *
 *	The Dragon and COCO have 18 x 256 byte sectors per track. We
 *	use them in pairs. We assume an even sectors per track. This is fine
 *	for our usage but would break for single density media.
 */

/* static uint8_t selmap[4] = { 0x01, 0x02, 0x04, 0x40 }; - COCO */
static uint8_t selmap[4] = {0x00, 0x01, 0x02, 0x03 };

static int fd_transfer(uint_fast8_t minor, bool is_read, uint_fast8_t rawflag)
{
    int tries;
    int count = 0;
    uint8_t err;
    uint8_t *driveptr = fd_tab + minor;
    uint8_t cmd[7];

    fd_motor_busy();		/* Touch the motor timer first so we don't
                                   go and turn it off as we are doing this */
    if (fd_selected != minor) {
        err = fd_motor_on(selmap[minor]);
        if (err)
            goto bad;
    }
    udata.u_nblock *= 2;

    if (rawflag == 1 && d_blkoff(BLKSHIFT))
        return -1;

    if (rawflag > 1)
            goto bad2;

//    kprintf("Issue command: drive %d\n", minor);
    cmd[0] = rawflag;
    cmd[1] = is_read ? FD_READ : FD_WRITE;
    cmd[2] = udata.u_block / 9;		/* 2 sectors per block */
    cmd[3] = ((udata.u_block % 9) << 1) + 1;	/*eww.. */
    cmd[4] = is_read ? OPDIR_READ: OPDIR_WRITE;
    cmd[5] = (uint16_t)udata.u_dptr >> 8;
    cmd[6] = (uint16_t)udata.u_dptr & 0xFF;
        
    while (count < udata.u_nblock) {
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
        cmd[3]++;	/* Next sector for next block */
        if (cmd[3] == 19) {
            cmd[3] = 1;	/* Track on */
            cmd[2]++;
        }
        count++;
    }
    fd_motor_idle();
    return count << 8;
bad:
    kprintf("fd%d: error %x\n", minor, err);
bad2:
    fd_motor_idle();
    udata.u_error = EIO;
    return -1;

}

int fd_open(uint_fast8_t minor, uint16_t flag)
{
    if(minor >= MAX_FD) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int fd_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
    return fd_transfer(minor, true, rawflag);
}

int fd_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
    return fd_transfer(minor, false, rawflag);
}

