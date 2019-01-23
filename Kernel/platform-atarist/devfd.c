#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devfd.h>

#define MAX_FD	4

#define OPDIR_NONE	0
#define OPDIR_READ	1
#define OPDIR_WRITE	2

#define FD_READ		0x88	/* 2797 needs 0x88, 1797 needs 0x80 */
#define FD_WRITE	0xA8	/* Likewise A8 v A0 */

static uint8_t motorct;
static uint8_t fd_selected = 0xFF;
static uint8_t fd_tab[MAX_FD];

static void fd_motor_busy(void)
{
    motorct++;
}

static void fd_motor_idle(void)
{
    motorct--;
    // if (motorct == 0) ... start timer */
}

static void fd_motor_timeout(void)
{
    fd_selected = 0xff;
//    fd_motor_off();
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

static int fd_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
    int ct = 0;
    int tries;
    uint8_t err;
    uint8_t *driveptr = fd_tab + minor;
    uint8_t cmd[6];

    if(rawflag == 2)
        goto bad2;

    fd_motor_busy();		/* Touch the motor timer first so we don't
                                   go and turn it off as we are doing this */
    if (fd_selected != minor) {
//        uint8_t err = fd_motor_on(selmap[minor]);
//        if (err)
//            goto bad;
    }

//    fd_map = rawflag;
    if (rawflag && d_blkoff(BLKSHIFT))
            return -1;

    udata.u_nblock *= 2;

//    kprintf("Issue command: drive %d\n", minor);
    cmd[0] = is_read ? FD_READ : FD_WRITE;
    cmd[1] = udata.u_block / 9;		/* 2 sectors per block */
    cmd[2] = ((udata.u_block % 9) << 1) + 1;	/*eww.. */
    cmd[3] = is_read ? OPDIR_READ: OPDIR_WRITE;
        
    while (ct < 2) {
        for (tries = 0; tries < 4 ; tries++) {
//            err = fd_operation(cmd, driveptr);
//            if (err == 0)
//                break;
//            if (tries > 1)
//                fd_reset(driveptr);
        }
        /* FIXME: should we try the other half and then bale out ? */
        if (tries == 3)
            goto bad;
        cmd[4]++;	/* Move on 256 bytes in the buffer */
        cmd[2]++;	/* Next sector for 2nd block */
        ct++;
    }
    fd_motor_idle();
    return 1;
bad:
    kprintf("fd%d: error %x\n", minor, err);
bad2:
    fd_motor_idle();
    udata.u_error = EIO;
    return -1;

}

int fd_open(uint8_t minor, uint16_t flag)
{
    if(minor >= MAX_FD) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int fd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    return fd_transfer(minor, true, rawflag);
}

int fd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    return fd_transfer(minor, false, rawflag);
}

