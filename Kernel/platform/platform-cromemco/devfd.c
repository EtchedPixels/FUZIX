#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devfd.h>

#define MAX_DRIVE	4

#define OPDIR_READ	0
#define OPDIR_WRITE	1

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
/* Step rate:
    5" 0E   slow 5" 0C   8" 0F   Slow 8" (special not supported) */

static uint8_t delay[MAX_DRIVE] = { 0x0F, 0x0F, 0x0F, 0x0F };
static uint8_t track[MAX_DRIVE] = { 0xFF, 0xFF, 0xFF, 0xFF };
/*
 *	We only support normal block I/O for the moment.
 */

static uint8_t selmap[MAX_DRIVE] = { 0x01, 0x02, 0x04, 0x08 };

static const uint8_t skewtab[2][18] = {
    /* Matches CDOS 5" skew for DSDD */
    { 1, 5, 9, 3, 7, 2, 6, 10, 4, 9 },
    /* No idea what CDOS uses for 8" DSDD right now */
    { 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15, 4, 8, 12, 16 },
};

static uint8_t live[4] = { 0xFF, 0xFF, 0xFF, 0xFF};

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
    int8_t tries;
    uint8_t err = 0;
    uint8_t drive = minor & 3;
    uint8_t trackno, sector;
    uint8_t large = !(minor & 0x10);
    const uint8_t *skew = skewtab[large];		/* skew table */
    uint16_t ct = 0;

    if(rawflag == 2)
        goto bad2;

    fd_map = rawflag;
    if (rawflag) {
        if (d_blkoff(BLKSHIFT))
            return -1;
        fd_map = udata.u_page;
    }

    /* Command to go to the controller after any seek is done */
    fd_cmd[0] = is_read ? FD_READ : FD_WRITE;
    /* Control byte: autowait, DD, motor, 5", drive bit */
    fd_cmd[1] = 0xE0 | selmap[drive];
    if (large)
        fd_cmd[1] |= 0x10;	/* turn on 8" bit */
    /* Directon of xfer */
    fd_cmd[2] = is_read ? OPDIR_READ: OPDIR_WRITE;
    fd_cmd[5] = 0x10 | delay[drive];

    /*
     *	Restore the track register to match this drive
     */
    if (track[drive] != 0xFF)
        fd_track = track[drive];
    else
        fd_reset();

    /*
     *	Begin transfers
     */
    while (ct < udata.u_nblock) {
        /* Need to consider SS v DS here */
        if (large) {
            fd_aux = 0x4C | ((udata.u_block & 16) ? 0 : 2) ;
            trackno = udata.u_block / 32;
            sector = udata.u_block % 16;
        } else {
            trackno = udata.u_block / 20;
            sector = udata.u_block % 20;
            if (sector > 9) {
                sector -= 10;
                fd_aux = 0x5C;	/* side 1 */
            } else
                fd_aux = 0x5E;
        }
        /* Buffer */
        fd_cmd[3] = ((uint16_t)udata.u_dptr) & 0xFF;
        fd_cmd[4] = ((uint16_t)udata.u_dptr) >> 8;

        for (tries = 0; tries < 4 ; tries++) {
            (void)fd_data;
            fd_sector = sector+1;/*FIXME/skew[sector];  Also makes 1 based */
            if (fd_track != trackno) {
                fd_data = trackno;
                if (fd_seek()) {
                    fd_reset();
                    continue;
                }
            }
            /* Do the read or write */
            err = fd_operation();
            if (err == 0)
                break;
            /* Try and recover */
            if (tries > 1)
                fd_reset();
        }
        if (tries == 4)
            goto bad;
        udata.u_block++;
        udata.u_dptr += 512;
        ct++;
    }
    /* Save the track */
    track[drive] = fd_track;
    return ct << 9;
bad:
    track[drive] = fd_track;
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

    if ((minor & 0xE0) || drivetype[minor & 3] == 255) {
        udata.u_error = ENODEV;
        return -1;
    }
    /* Ensure we can't open the same physical disk in two modes at once */
    if (*dp != 0xFF && *dp != minor) {
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

    