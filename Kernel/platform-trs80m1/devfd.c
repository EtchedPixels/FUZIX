/*
 *	Higher level logic for TRS80 floppy disk interfaces and clones
 *	on both Model 1 and Model 3
 *
 *	Things To Do
 *	- Set sector size and shift values in the config calls
 *	- Use those in the I/O loop so we can do varying sector sizes
 *	- Handle double sided media (need to consider heads in the loop)
 *	- Turn the step rate value into a step mask and use it in the asm code
 *	- Teach the asm code about density
 *	- Teach the asm code about double sided and maybe 128 byte sectors
 *	- Rework density handling
 *	- Write compensation
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
#include <trs80.h>

#define MAX_FD	4

#define OPDIR_NONE	0
#define OPDIR_READ	1
#define OPDIR_WRITE	2

#define FD_READ		0x80	/* 2797 needs 0x88, 1797 needs 0x80 */
#define FD_WRITE	0xA0	/* Likewise A8 v A0 */

static uint8_t motorct;

/* Extern as they live in common */
extern uint8_t fd_map, fd_tab[MAX_FD];
extern uint8_t fd_selected;
extern uint8_t fd_cmd[7];

static struct fd_ops *fops;

static struct fd_ops fd1_ops = {
    fd_restore,
    fd_operation,
    fd_motor_on
};

static struct fd_ops fd3_ops = {
    fd3_restore,
    fd3_operation,
    fd3_motor_on
};

static struct fdcinfo fdcap[MAX_FD] = {
    {
        FDF_SD|FDF_DD|FDF_DS|FDF_SEC256|FDF_SEC512,
        0,
        80,
        2,
        12,
        FDC_DSTEP|FDC_AUTO|FDC_SEC0,
        FDC_FMT_17XX,
        0, /* To calc worst case */
        0,0,0
    },
    {
        FDF_SD|FDF_DD|FDF_DS|FDF_SEC256|FDF_SEC512,
        0,
        80,
        2,
        12,
        FDC_DSTEP|FDC_AUTO|FDC_SEC0,
        FDC_FMT_17XX,
        0, /* To calc worst case */
        0,0,0
    },
    {
        FDF_SD|FDF_DD|FDF_DS|FDF_SEC256|FDF_SEC512,
        0,
        80,
        2,
        12,
        FDC_DSTEP|FDC_AUTO|FDC_SEC0,
        FDC_FMT_17XX,
        0, /* To calc worst case */
        0,0,0
    },
    {
        FDF_SD|FDF_DD|FDF_DS|FDF_SEC256|FDF_SEC512,
        0,
        80,
        2,
        12,
        FDC_DSTEP|FDC_AUTO|FDC_SEC0,
        FDC_FMT_17XX,
        0, /* To calc worst case */
        0,0,0
    },
};

static struct fdcinfo fdc[MAX_FD] = {
    {
        FDF_DD|FDF_SEC512,
        0,
        40,
        1,
        9,
        0,
        FDC_FMT_17XX,
        0, /* To calc worst case */
        0,0,0
    },
    {
        FDF_DD|FDF_SEC512,
        0,
        40,
        1,
        9,
        0,
        FDC_FMT_17XX,
        0, /* To calc worst case */
        0,0,0
    },
    {
        FDF_DD|FDF_SEC512,
        0,
        40,
        1,
        9,
        0,
        FDC_FMT_17XX,
        0, /* To calc worst case */
        0,0,0
    },
    {
        FDF_DD|FDF_SEC512,
        0,
        40,
        1,
        9,
        0,
        FDC_FMT_17XX,
        0, /* To calc worst case */
        0,0,0
    },
};

/* Translate the drive into a selection. Assumes single sided on the M1 */
static uint8_t selmap[4] = { 0x01, 0x02, 0x04, 0x08 };

static int fd_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
    int ct = 0;
    int tries;
    uint8_t err = 0;
    uint8_t *driveptr = fd_tab + minor;
    struct fdcinfo *f = fdc + minor;

    /* You can't swap to floppy */
    if(rawflag == 2)
        goto bad2;

    /* Do we need to select the drive ? */
    if (trs80_model != TRS80_MODEL3 || fd_selected != minor) {
        uint8_t err;
        uint8_t tmp = 0;
        /* Decide if we need double density */
        if (f->features & FDF_DD)
            tmp |= 0x80;
        err = fops->fd_motor_on(selmap[minor]|tmp);
        if (err)
            goto bad;
    }

    /* If we don't know where the head on this drive is then force
       a seek */
    if (*driveptr == 0xFF)
        if (err = fops->fd_restore(driveptr))
            goto bad;

    /* Adjust for the block size if raw I/O. For now hard code 512 byte
       sectors but we need to sort this */
    fd_map = rawflag;
    if (rawflag && d_blkoff(BLKSHIFT))
            return -1;

    /* We only deal with single sided 512 byte/sector media for the moment */
    fd_cmd[0] = is_read ? FD_READ : FD_WRITE;
    fd_cmd[3] = is_read ? OPDIR_READ: OPDIR_WRITE;
    fd_cmd[4] = ((uint16_t)udata.u_dptr) & 0xFF;
    fd_cmd[5] = ((uint16_t)udata.u_dptr) >> 8;
    fd_cmd[6] = 2; /* 0 128 1 256 2 512 : fixed for now */

    while (ct < udata.u_nblock) {
        /* For each block we need to load we work out where to find it */
        fd_cmd[1] = udata.u_block / f->sectors;
        fd_cmd[2] = udata.u_block % f->sectors;
        /* Some single density media has sectors numbered from zero */
        if (!(f->config & FDC_SEC0))
            fd_cmd[2]++;
        /* Reading 40 track media on an 80 track drive */
        if (f->config & FDC_DSTEP)
            fd_cmd[1] <<= 1;
        /* Now try the I/O */
        for (tries = 0; tries < 4 ; tries++) {
            err = fops->fd_op(driveptr);
            if (err == 0)
                break;
            /* Reposition the head */
            if (tries > 1)
                fops->fd_restore(driveptr);
        }
        if (tries == 4)
            goto bad;
        fd_cmd[5]++;	/* Move on 256 bytes in the buffer */
        fd_cmd[5]++;	/* Move on 256 bytes in the buffer */
        ct++;
    }
    return udata.u_nblock << 9;
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
    /* No media ? */
    if (fops->fd_restore(fd_tab + minor) && !(flag & O_NDELAY)) {
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
    switch(request) {
        case FDIO_GETCAP:
            return uput(fdcap + minor, buffer, sizeof(struct fdcinfo));
        case FDIO_GETINFO:
            return uput(fdc + minor, buffer, sizeof(struct fdcinfo));
        case FDIO_SETINFO:
            /* Ick.. but we are not portable code so we know how it packs */
            if (uget(fdc + minor, buffer, 7))
                return -1;
            /*FIXME when we sort DS meida */
            fdc[minor].heads = 1;
            /* TODO : steprate to masks */
            fdc[minor].features &= fdcap[minor].features;
            /* Force reconfiguration */
            fd_tab[minor] = 0xFF;
            fd_selected = 255;
            return 0;
        case FDIO_FMTTRK:
            return -1;
        case FDIO_RESTORE:
            return fops->fd_restore(fd_tab + minor);
    }
    return -1;
}

void floppy_setup(void)
{
    if (trs80_model == TRS80_MODEL3)
        fops = &fd3_ops;
    else
        fops = &fd1_ops;
}
