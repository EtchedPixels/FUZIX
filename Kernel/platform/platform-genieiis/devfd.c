/*
 *	Higher level logic for Genie IIs floppy (much like TRS80)
 *
 *	Things To Do
 *	- FIXME: genieIIs differences
 *	- Teach the asm code about density
 *	- Teach the asm code about double sided and maybe 128 byte sectors
 *	- Rework density handling
 *	- Formatting
 *	- Motor delay improvements
 *	- Try slowing step rate on repeated errors ?
 *	- 4 retries may not be sufficient ?
 *	- Head jiggling before we try restore ?
 *	- Tandy style doubler - how to pass that info ?
 *	- Autodetect density/sides/geometry
 */
#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <fdc.h>
#include <devfd.h>
#include <genie.h>

#define MAX_FD	4

#define OPDIR_NONE	0
#define OPDIR_READ	1
#define OPDIR_WRITE	2

#define FD_READ		0x80
#define FD_WRITE	0xA0

static uint8_t motorct;

/* Extern as they live in common */
extern uint8_t fd_map, fd_tab[MAX_FD];
extern uint8_t fd_selected;
extern uint8_t fd_cmd[10];

static struct fd_ops *fops;

static struct fd_ops fd_ops = {
    fd_restore,
    fd_operation,
    fd_motor_on
};

static struct fdcinfo fdcap = {
        0,
        0,
        FDC_DSTEP|FDC_SEC0|FDC_PRECOMP,	/* Not all done yet */
        FDF_DD|FDF_DS|FDF_SEC256|FDF_SEC512,
        18,
        40,
        2,			/* Actually most are single sided */
        0			/* Precomp */
};

static uint8_t mode[MAX_FD];

#define NUM_MODES	5
static struct fdcinfo fdcmodes[NUM_MODES] = {
    /* Classic TRS80 SS/DD */
    {
        0,
        FDTYPE_TRS40DD,
        FDF_DD|FDF_SEC256,	/* Double density 256 byte sectors */
        18,			/* 18 sectors/track */
        40,			/* 40 tracks */
        1,			/* single sided */
        0
    },
    /* Classic TRS80 DS/DD: late machines only */
    {
        1,
        FDTYPE_TRS40DD,
        FDF_DD|FDF_SEC256,	/* Double density 256 byte sectors */
        18,			/* 18 sectors/track */
        40,			/* 40 tracks */
        2,			/* double sided */
        0
    },
    /* TRS80 Model 1 compatible images. Will not work with TRSDOS as TRSDOS
       uses weird DAM values. Should be fine with Fuzix<->Fuzix */
    {
        2,
        FDTYPE_TRS40SD,
        FDF_DD|FDF_SEC256,	/* Single density 256 byte sectors */
        10,			/* 10 sectors/track */
        40,			/* 40 tracks */
        1,			/* double sided */
        21
    },
    {
        3,
        FDTYPE_PC360,		/* Double sided drive, PC format */
        FDF_DD|FDF_SEC512,
        9,
        40,
        2,
        0
    },
    {
        4,
        FDTYPE_PC180,		/* Interchange with PC and TRS80 SS drive */
        FDF_DD|FDF_SEC512,
        9,
        40,
        1,
        0
    }
};

struct diskprop {
    uint8_t shift;
    uint16_t size;
    uint8_t precomp;
    uint8_t features;
    uint8_t sectors;	/* per track */
    uint8_t heads;
    uint8_t step;
};


static struct diskprop diskprop[MAX_FD];

/* Translate the drive into a selection. Assumes single sided on the M1 */
static uint8_t selmap[MAX_FD] = { 0x01, 0x02, 0x04, 0x08 };

static uint8_t fd_select(uint8_t minor)
{
    uint8_t err = 0;
    uint8_t tmp = 0;
    /* Decide if we need double density */
    if (diskprop[minor].features & FDF_DD)
        tmp |= 0x80;
    err = fd_ops.fd_motor_on(selmap[minor]|tmp);
    if (!err)
        fd_selected = minor;
    return err;
}

static uint8_t do_fd_restore(uint8_t minor)
{
    if (fd_select(minor))
        return 0xFF;
    return fd_ops.fd_restore(fd_tab + minor);
}

static int fd_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
    int ct = 0;
    int tries;
    uint8_t err = 0;
    struct diskprop *dp = diskprop + minor;
    uint8_t *driveptr = fd_tab + minor;

    /* You can't swap to floppy */
    if(rawflag == 2)
        goto bad2;

    /* Do we need to select the drive ? */
    if (fd_select(minor))
            goto bad;

    /* If we don't know where the head on this drive is then force
       a seek */
    if (*driveptr == 0xFF) {
        if (err = fd_ops.fd_restore(driveptr))
            goto bad;
    }

    fd_map = rawflag;
    if (rawflag) {
        if (d_blkoff(9 - dp->shift))
            return -1;
    } else {
        udata.u_nblock <<= dp->shift;
        udata.u_block <<=  dp->shift;
    }

    fd_cmd[0] = is_read ? FD_READ : FD_WRITE;
    fd_cmd[3] = is_read ? OPDIR_READ: OPDIR_WRITE;
    fd_cmd[6] = dp->shift;
    fd_cmd[7] = dp->step;
    fd_cmd[8] = dp->precomp;

    while (ct < udata.u_nblock) {
        /* For each block we need to load we work out where to find it */
        fd_cmd[1] = udata.u_block / dp->sectors;
        fd_cmd[2] = udata.u_block % dp->sectors;
        if (dp->heads == 2) {
            fd_cmd[9] = fd_cmd[1] & 1;
            fd_cmd[1] >>= 1;
        } else
            fd_cmd[9] = 0;
        fd_cmd[4] = ((uint16_t)udata.u_dptr) & 0xFF;
        fd_cmd[5] = ((uint16_t)udata.u_dptr) >> 8;
        /* Some single density media has sectors numbered from zero */
        if (!(dp->features & FDC_SEC0))
            fd_cmd[2]++;
        /* Now try the I/O */
        for (tries = 0; tries < 4 ; tries++) {
            err = fd_ops.fd_op(driveptr);
            if (err == 0)
                break;
            if (!is_read && err != 0xFF && (err & 0x40)) {
                udata.u_error = EROFS;
                return -1;
            }
            /* Reposition the head */
            if (tries > 1)
                fd_ops.fd_restore(driveptr);
        }
        if (tries == 4)
            goto bad;
        udata.u_dptr += dp->size;
        udata.u_block++;
        ct++;
    }
    return udata.u_nblock << (7 + dp->shift);
bad:
    kprintf("fd%d: error %x\n", minor, err);
bad2:
    udata.u_error = EIO;
    return -1;
}

static void fd_setup(uint8_t minor, uint8_t step)
{
    /* Copy the features for the mode into the device parameters */
    struct fdcinfo *f = fdcmodes + mode[minor];
    struct diskprop *dp = diskprop + minor;
    uint8_t s;

    dp->features = f->features;
    dp->size = f->features & FDF_SECSIZE;
    switch(dp->size) {
    case 128:
        s = 0;
        break;
    case 256:
    default:
        s = 1;
        break;
    case 512:
        s = 2;
        break;
    }
    dp->shift = s;
    dp->sectors = f->sectors;
    dp->heads = f->heads;
    if (dp->features & FDF_DD)
        dp->precomp = 21;		/* DD 35/40 track drives */
    else
        dp->precomp = 255;		/* None */
    if (step != 255)
        dp->step = step;
    /* Force reconfiguration */
    fd_tab[minor] = 0xFF;
    fd_selected = 255;
}

int fd_open(uint8_t minor, uint16_t flag)
{
    flag;
    if(minor >= MAX_FD) {
        udata.u_error = ENODEV;
        return -1;
    }
    fd_setup(minor, 255);
    /* No media ? */
    if (do_fd_restore(minor) && !(flag & O_NDELAY)) {
        udata.u_error = EIO;
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

int fd_ioctl(uint8_t minor, uarg_t request, char *buffer)
{
    uint8_t s;
    struct fdcstep step;

    switch(request) {
        case FDIO_GETCAP:
            fdcap.mode = mode[minor];
            return uput(&fdcap, buffer, sizeof(struct fdcinfo));
        case FDIO_GETMODE:
            s = ugetc(buffer);
            if (s >= NUM_MODES) {
                udata.u_error = EINVAL;
                return -1;
            }
            return uput(fdcmodes + s, buffer, sizeof(struct fdcinfo));
        case FDIO_SETMODE:
            s = ugetc(buffer);
            if (s >= NUM_MODES) {
                udata.u_error = EINVAL;
                return -1;
            }
            mode[minor] = s;
            fd_setup(minor, 2);
            return 0;
        case FDIO_SETSTEP:
            if (uget(buffer, &step, sizeof(step)))
                return -1;
            s = step.steprate;
            /* Check chip and clock details */
            /* WD1771 - 6 6 10 20 */
            if (s >= 20)
                s = 3;
            else if (s >= 10)
                s = 2;
            else
                s = 0;
            /* TODO: head load and settle */
            fd_setup(minor, s);
            return 0;
        case FDIO_FMTTRK:
            /* TODO */
            return -1;
        case FDIO_RESTORE:
            fd_selected = 255;	/* Force a re-configure */
            return do_fd_restore(minor);
    }
    return -1;
}

void floppy_setup(void)
{
}
