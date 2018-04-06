#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devfd.h>

#define MAX_DRIVE	4

#define OPDIR_NONE	0
#define OPDIR_READ	1
#define OPDIR_WRITE	2

#define FD_READ		0x80	/* 2797 needs 0x88, 1797 needs 0x80 */
#define FD_WRITE	0xA0	/* Likewise A8 v A0 */

/* Extern as they live in common */
extern uint8_t fd_map;
extern uint8_t fd_selected;
extern uint8_t fd_cmd[7];

static uint8_t fd_tab[MAX_DRIVE];

__sfr __at 0x04	fd_aux;
__sfr __at 0x30 fd_stcmd;
__sfr __at 0x31 fd_track;
__sfr __at 0x32 fd_sector;
__sfr __at 0x33 fd_data;
__sfr __at 0x34 fd_ctrlflag;

/* 4 large disks (0 5" 1 8" 255 none */

uint8_t drivetype[4] = { 1, 1, 1, 1 };

/*
 *	We only support normal block I/O for the moment.
 */

static uint8_t selmap[4] = { 0x01, 0x02, 0x04, 0x08 };

static const uint8_t skewtab[2][18] = {
    /* Matches CDOS 5" skew for DSDD */
    { 1, 5, 9, 3, 7, 2, 6, 10, 4, 9 },
    /* No idea what CDOS uses for 8" DSDD right now */
    { 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15, 4, 8, 12, 16 },
};

static uint8_t live[4];

/*
 *	A note on disk formats and the Cromenco (and also emulator limits)
 *
 *	Classic 8" SS/SD	26 sectors, 128 bytes/sector, 77 track
 *				software skewed
 *	Classic 5" SS/SD	18 sectors, 40 tracks otherwise as before
 *
 *	Double sided use the same formats for SD but with two sides
 *
 *	When we go to DD 8" goes to 16 512 byte sectors/track and
 *	5" goes to 10 512 byte sectors/track
 *
 *	The controller can do a lot more but the emulators don't always
 *	cope. In particular a 'correct' bootable disk is supposed to be
 *	SD on track 0 side 0 and can be DS/DD on the rest.
 *
 *	Likewise the hardware can write PC format media in theory and
 *	do format level skewing at least for 5.25" disks
 *
 *	Just to get us going - only do DSDD.
 */
 
static int fd_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
    int tries;
    uint8_t err = 0;
    uint8_t drive = minor & 3;
    uint8_t *driveptr = fd_tab + drive;
    uint8_t large = !(minor & 0x10);
    const uint8_t *skew = skewtab[large];		/* skew table */

    if(rawflag == 2)
        goto bad2;

    fd_map = rawflag;
    if (rawflag && d_blkoff(BLKSHIFT))
            return -1;

    /* Command to go to the controller after any seek is done */
    fd_cmd[0] = is_read ? FD_READ : FD_WRITE;
    if (large) {
        /* Track */
        fd_cmd[1] = udata.u_block / 16;
        /* Sector */
        fd_cmd[2] = skew[udata.u_block % 16];	/* 1..n skewed */
        /* control */
        fd_cmd[6] = 0xF0;
    } else {
        fd_cmd[1] = udata.u_block / 10;
        fd_cmd[2] = skew[udata.u_block % 10];
        fd_cmd[6] = 0xA0;
    }
    fd_cmd[6] |= (1 << drive);
    /* Directon of xfer */
    fd_cmd[3] = is_read ? OPDIR_READ: OPDIR_WRITE;
    /* Buffer */
    fd_cmd[4] = ((uint16_t)udata.u_dptr) & 0xFF;
    fd_cmd[5] = ((uint16_t)udata.u_dptr) >> 8;
    
    /* FIXME: Sides. Really 32 sectors/track 16 each side */

    while (udata.u_done < udata.u_nblock) {
        if (large) {
            fd_cmd[1] = udata.u_block / 16;
            fd_cmd[2] = (udata.u_block % 16) + 1;
        } else {
            fd_cmd[1] = udata.u_block / 10;
            fd_cmd[2] = (udata.u_block % 10) + 1;
        }
        for (tries = 0; tries < 4 ; tries++) {
            err = fd_operation(driveptr);
            if (err == 0)
                break;
            if (tries > 1)
                fd_reset(driveptr);
        }
        if (tries == 4)
            goto bad;
        udata.u_block++;
        udata.u_done++;
    }
    return udata.u_done << 9;
bad:
    kprintf("fd%d: error %x\n", minor, err);
bad2:
    udata.u_error = EIO;
    return -1;
}

/* We encode the minor has
        000[8/5][sd/dd][ss/ds][disk.2] */

int fd_open(uint8_t minor, uint16_t flag)
{
    uint8_t *dp = live + (minor & 3);

    flag;
    if ((minor & 0xE0) || drivetype[minor] == 255) {
        udata.u_error = ENODEV;
        return -1;
    }
    /* Ensure we can't open the same physical disk in two modes at once */
    if (*dp != 0xFF) {
        udata.u_error = EBUSY;
        return -1;
    }
    *dp = minor;
    return 0;
}

int fd_close(uint8_t minor)
{
    live[minor & 3] = 0xFF;
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

    