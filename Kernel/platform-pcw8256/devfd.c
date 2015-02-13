/* 
 *	Amstrad PCW8256 Floppy Driver
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devfd.h>

__sfr __at 0xf8 asic_ctrl;
__sfr __at 0x00 fdc_c;
__sfr __at 0x01 fdc_d;

static int fd_transfer(bool is_read, uint8_t minor, uint8_t rawflag);

static uint8_t track[2];
static uint8_t type[2];
#define TYPE_NONE	0
#define TYPE_THREE	1
#define TYPE_THREE_FIVE	2
static uint8_t motorct;
static int8_t devsel = -1;

int fd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    return fd_transfer(true, minor, rawflag);
}

int fd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    return fd_transfer(false, minor, rawflag);
}

static int fd_send(uint8_t cmd, int minor)
{
    fd765_cmdbuf[1] = minor;
    fd765_cmdbuf[0] = cmd;
    return fd765_cmd2();
}

static void reset_motor_timer(int minor)
{
    minor;
}

static void motor_on(int minor)
{
    minor;
    asic_ctrl = 0x09;
//FIXME    while(!(fd_send(0x04, minor) & 0x20));
    kprintf("Motor on %d\n", minor);
}

static void motor_off(void)
{
    asic_ctrl = 0x0A;
    devsel = -1;
}

/*
 *	Turn the block number into geometry and poke it into the
 *	command buffers
 */
static void fd_geom(int minor, blkno_t block)
{
    int ntrack = block / 9;
    int nsector = (block % 9) + 1;
    fd765_cmdbuf[2] = ntrack;
    fd765_rw_data[2] = ntrack;
    fd765_rw_data[3] = 0;		/* single sided for now */
    fd765_rw_data[4] = nsector;
    fd765_rw_data[6] = nsector;
    reset_motor_timer(minor);

    kprintf("fd(%d,%d)\n", ntrack, nsector);
    if (ntrack == track[minor])
        return;
    fd765_cmdbuf[0] = 0x0F;
    fd765_cmd3();

    if (fd765_intwait() & 0x20)
        track[minor] = fd765_statbuf[1] & 0x7F;

}

/*
 *	Select a driver, ensure the motor is on and we are ready
 *	then set up the command buffers to reflect this device
 */
static void fd_select(int minor)
{
    if (devsel == minor)
        return;
    motor_on(minor);
    fd765_cmdbuf[1] = minor;
    fd765_rw_data[1] = minor;
}

/*
 *	Block transfer
 */
static int fd_transfer(bool is_read, uint8_t minor, uint8_t rawflag)
{
    blkno_t block;
    int block_xfer;     /* r/w return value (number of 512 byte blocks transferred) */
    uint16_t dptr;
    int dlen;
    int ct = 0;
    int st;
    int tries;

    /* Direct to user space, or kernel buffered I/O ? */
    if(rawflag) {
        dlen = udata.u_count;
        dptr = (uint16_t)udata.u_base;
        block = udata.u_offset >> BLKSHIFT;
        block_xfer = dlen >> 9;
    } else { /* rawflag == 0 */
        dlen = 512;
        dptr = (uint16_t)udata.u_buf->bf_data;
        block = udata.u_buf->bf_blk;
        block_xfer = 1;
    }
    
    fd_select(minor);		/* Select, motor on */
    fd765_user = rawflag;	/* Tell the asm which map */
//    kprintf("To %d:%x for %d\n", rawflag, dptr, block_xfer);
    while (ct < block_xfer) {	/* For each block */
        fd_geom(minor, block);	/* Map it and set the geometry */
        fd765_buffer = dptr;
        for (tries = 0; tries < 3; tries ++) {	/* Try 3 times */
            if (tries != 0)			/* After a fail recalibrate */
                fd_send(0x07, minor);
            if (is_read) {
                fd765_rw_data[0] = 0x66;	/* MFM 512 byte read */
                st = fd765_read_sector();
            } else {
                fd765_rw_data[0] = 0x65;	/* MFM 512 byte write */
                st = fd765_write_sector();
            }
            /* Did it work ? */
            if (st == 0)
                break;
        }
        if (tries == 3) {
            kprintf("fd%d: I/O error %d:%d\n", is_read, block);
            if (rawflag)
                break;
        }
        block++;
        ct++;
        dptr += 512;
    }
    return ct;
}

int fd_open(uint8_t minor, uint16_t flag)
{
    flag;
    if(minor > 1 || !type[minor]) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}


static void fd_probe_plus(int d)
{
    int du = 2 * d;
    uint8_t st;
    motor_on(d);
    st = fd_send(0x07, du);
    st = fd765_intwait();
    st = fd_send(0x04, du);
    if (!(st & 0x10)) {
        if (st & 0x40)
            type[d] = TYPE_THREE;
        else if (d == 0)
            type[d] = TYPE_THREE_FIVE;
        else
            type[d] = TYPE_NONE;
    } else {
        asic_ctrl = 0x10;
        /* motor wait needed */
        fd_send(0x04, du);
        if (st & 0x10)
            type[d] = TYPE_THREE;
        else
            type[d] = TYPE_THREE_FIVE;
    }
    /* Motors back off */
    motor_off();
}

static char *fdnames[] = {"none", "3\"", "3.5\"" };

void fd_probe(void)
{
    int i;
    uint8_t st;
    /* Motor off */
    motor_off();
    /* Wait for not ready status */
    do {
        st = fd_send(0x04, 0);
    } while (st & 0x20);
    st = fd_send(0x04, 3);
    if (st & 0x20) {
        fd_probe_plus(0);
        fd_probe_plus(1);
    }
    else
        type[0] = TYPE_THREE;
    motor_off();
    for (i = 0; i < 2; i++)
        kprintf("fd%d: %s\n", i, fdnames[type[i]]);
}
