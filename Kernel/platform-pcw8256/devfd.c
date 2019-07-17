/* 
 *	Amstrad PCW8256 Floppy Driver
 *
 *	FIXME: unify with generic devfd765 code
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devfd.h>

__sfr __at 0xf8 asic_ctrl;
__sfr __at 0x00 fdc_c;
__sfr __at 0x01 fdc_d;

static int fd_transfer(bool is_read, uint8_t minor, uint8_t rawflag);

static uint8_t track[2] = { 255, 255 };
static uint8_t sides[2];
static uint8_t type[2];
#define TYPE_NONE	0
#define TYPE_THREE	1
#define TYPE_THREE_FIVE	2
#define TYPE_UNKNOWN	3
#define TYPE_DS		4

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
    uint8_t nsector = block % 9;
    uint8_t ntrack = block / 9;
    uint8_t side = 0;

    nsector++;
    /* These two will eventually need to be according to media type */
    side = ntrack & 1;
    ntrack >>= 1;

    fd765_cmdbuf[2] = ntrack;
    fd765_rw_data[1] = (side << 2) | minor;
    fd765_rw_data[2] = ntrack;
    fd765_rw_data[3] = side;
    fd765_rw_data[4] = nsector;
    fd765_rw_data[6] = nsector;
    reset_motor_timer(minor);

    if (ntrack == track[minor])
        return;
    fd765_cmdbuf[0] = 0x0F;
    fd765_cmdbuf[1] = (side << 2) | minor;
    fd765_intwait();
    fd765_cmd3();

    if (fd765_intwait() & 0x20)
        track[minor] = ntrack;//FIXME??fd765_statbuf[1] & 0x7F;
    else
        track[minor] = 0xFF;
}

/*
 *	Select a drive, ensure the motor is on and we are ready
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
    uint16_t dptr;
    int ct = 0;
    int st;
    int tries;

    if (rawflag == 2) {
        udata.u_error = EIO;
        return -1;
    }

    /* Direct to user space, or kernel buffered I/O ? */
    if(rawflag && d_blkoff(BLKSHIFT))
        return -1;

    dptr = (uint16_t)udata.u_dptr;
    
    fd_select(minor);		/* Select, motor on */
    fd765_user = rawflag;	/* Tell the asm which map */
    while (ct < udata.u_nblock) {	/* For each block */
        fd765_buffer = dptr;
        for (tries = 0; tries < 3; tries ++) {	/* Try 3 times */
            if (tries != 0) {			/* After a fail recalibrate */
                fd_send(0x07, minor);
                track[minor] = 0;
            }
            fd_geom(minor, udata.u_block);	/* Map it and set the geometry */
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
            kprintf("fd%d: I/O error %d:%d\n", is_read, udata.u_block);
            if (rawflag)
                break;
        }
        udata.u_block++;
        ct++;
        dptr += 512;
    }
    return ct ? (ct << BLKSHIFT) : - 1;
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

/* FIXME: Want an fdc discard */

static void fd_probe_plus(int d)
{
    int du = 2 * d;
    uint8_t st;
    motor_on(d);
    st = fd_send(0x07, du);
    st = fd765_intwait();
    st = fd_send(0x04, du);
    /* Check track 0 */
    /* Did it report trk0 */
    if (!(st & 0x10)) {
        /* Not set: r/o means 3 r/w 3.5, for slave means no drive */
        if (st & 0x40)
            type[d] = TYPE_THREE;
        else if (d == 0)
            type[d] = TYPE_THREE_FIVE;
        else
            type[d] = TYPE_NONE;
    } else {
        asic_ctrl = 0x10;
        /* motor wait needed */
        motor_off();
        /* Wait for not ready status */
        do {
            st = fd_send(0x04, 0);
        } while (st & 0x20);
        st = fd_send(0x04, du);
        if (st & 0x10)
            type[d] = TYPE_THREE;
        else
            type[d] = TYPE_THREE_FIVE;
    }
    if (st & 8)
        type[d] |= TYPE_DS;
    /* Motors back off */
    motor_off();
}

static const char *fdnames[] = {
    NULL,
    "180K",
    "3.5\"",
    "??",
    NULL,
    "720K",
    "3.5\"",
    "??",
    NULL
};

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
    /* Get status of 'drive 2' */
    st = fd_send(0x04, 2);
    if (st & 0x20) {
        /* Reports ready - new style drive */
        fd_probe_plus(0);
        fd_probe_plus(1);
    }
    else {
        /* FIXME: can we detect 3.5" on older machines. Need to play about */
        st = fd_send(0x04, 0);
        type[0] = TYPE_THREE | ((st & 8) ? TYPE_DS : 0);
        motor_on(0);
        st = fd_send(0x07, 1);
        st = fd765_intwait();
        st = fd_send(0x04, 1);
        if (st & 0x10) {
            type[1] = TYPE_THREE | ((st & 8) ? TYPE_DS : 0);
            sides[1] = (st & 8) ? 2 : 1;
        }
        /* FIXME: should do the 3 or 3.5 check generically - 3.5 won't
           trk0 if not ready */
    }
    motor_off();
    for (i = 0; i < 2; i++) {
        const char *p = fdnames[type[i]];
        if (p == NULL) {
            type[i] = TYPE_NONE;
            continue;
        }
        if (type[i] & TYPE_DS)
            sides[i] = 2;
        else
            sides[i] = 1;
        kprintf("fd%d: %s\n", i, p);
    }
}
