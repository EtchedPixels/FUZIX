/*
 *	Things To Do
 *	- Handle double sided media (need to consider heads in the loop)
 *	- Teach the asm code about density
 *	- Teach the asm code about double sided and maybe 128 byte sectors
 *	- Rework density handling
 *	- Formatting
 *	- Motor delay improvements
 *	- Try slowing step rate on repeated errors ?
 *	- 4 retries may not be sufficient ?
 *	- Head jiggling before we try restore ?
 *	- Autodetect density/sides/geometry
 */
#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <fdc.h>
#include <devfd.h>

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
extern uint8_t fd_cmd[9];

static struct fdcinfo fdcap[MAX_FD] = {
    {
        FDF_DD|FDF_DS|FDF_SEC256|FDF_SEC512,
        0,
        80,
        2,
        12,
        0,			/* TODO precomp */
        0,			/* unused */
        FDC_DSTEP|FDC_SEC0,
        FDC_FMT_17XX,
        0, /* To calc worst case */
        0,0
    },
    {
        FDF_DD|FDF_DS|FDF_SEC256|FDF_SEC512,
        0,
        80,
        2,
        12,
        0,			/* TODO precomp */
        0,			/* unused */
        FDC_DSTEP|FDC_SEC0,
        FDC_FMT_17XX,
        0, /* To calc worst case */
        0,0
    },
    {
        FDF_DD|FDF_DS|FDF_SEC256|FDF_SEC512,
        0,
        80,
        2,
        12,
        0,			/* TODO precomp */
        0,			/* unused */
        FDC_DSTEP|FDC_SEC0,
        FDC_FMT_17XX,
        0, /* To calc worst case */
        0,0
    },
    {
        FDF_DD|FDF_DS|FDF_SEC256|FDF_SEC512,
        0,
        80,
        2,
        12,
        0,			/* TODO precomp */
        0,			/* unused */
        FDC_DSTEP|FDC_SEC0,
        FDC_FMT_17XX,
        0, /* To calc worst case */
        0,0
    }
};

static struct fdcinfo fdc[MAX_FD] = {
    {
        FDF_DD|FDF_SEC256,	/* Double density 256 byte sectors */
        18,			/* 18 sectors/track */
        40,			/* 40 tracks */
        1,			/* single sided */
        12,			/* steprate */
        0,			/* no precomp */
        0,			/* unused */
        0,			/* config default */
        FDC_FMT_17XX,
        0,
        0,0
    },
    {
        FDF_DD|FDF_SEC256,	/* Double density 256 byte sectors */
        18,			/* 18 sectors/track */
        40,			/* 40 tracks */
        1,			/* single sided */
        12,			/* steprate */
        0,			/* no precomp */
        0,			/* unused */
        0,			/* config default */
        FDC_FMT_17XX,
        0,
        0,0
    },
    {
        FDF_DD|FDF_SEC256,	/* Double density 256 byte sectors */
        18,			/* 18 sectors/track */
        40,			/* 40 tracks */
        1,			/* single sided */
        12,			/* steprate */
        0,			/* no precomp */
        0,			/* unused */
        0,			/* config default */
        FDC_FMT_17XX,
        0,
        0,0
    },
    {
        FDF_DD|FDF_SEC256,	/* Double density 256 byte sectors */
        18,			/* 18 sectors/track */
        40,			/* 40 tracks */
        1,			/* single sided */
        12,			/* steprate */
        0,			/* no precomp */
        0,			/* unused */
        0,			/* config default */
        FDC_FMT_17XX,
        0,
        0,0
    }
};

/* Consider a struct of  this plus the fdcinfo to make the referencing
   nice ? */
static uint8_t shift[MAX_FD] = { 1, 1, 1, 1 };
static uint16_t size[MAX_FD] = { 256, 256, 256, 256 };
static uint8_t step[MAX_FD] = { 2, 2, 2, 2 };

/*
 *	We only support normal block I/O for the moment. We do need to
 *	add swapping!
 */
static uint8_t selmap[MAX_FD] = { 0x01, 0x02, 0x04, 0x08 };

static uint8_t fd_select(uint8_t minor)
{
    uint8_t err = 0;
    uint8_t tmp = 0;
    /* Do we need to select the drive ? */
    if (fd_selected != minor) {
        /* Decide if we need double density */
        if (fdc[minor].features & FDF_DD)
            tmp |= 0x80;
        err = fd_motor_on(selmap[minor]|tmp);
        if (!err)
            fd_selected = minor;
    }
    return err;
}

static uint8_t do_fd_restore(uint8_t minor)
{
    if (fd_select(minor))
        return 0xFF;
    return fd_restore(fd_tab + minor);
}

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

    if (fd_select(minor))
            goto bad;

    if (*driveptr == 0xFF)
        if (err = fd_restore(driveptr))
            goto bad;

    fd_map = rawflag;
    if (rawflag) {
        if (d_blkoff(9 - shift[minor]))
            return -1;
    } else {
        udata.u_nblock <<= shift[minor];
        udata.u_block <<= shift[minor];
    }

    kprintf("Want block %u\n", udata.u_block);
    fd_cmd[0] = is_read ? FD_READ : FD_WRITE;
    fd_cmd[3] = is_read ? OPDIR_READ: OPDIR_WRITE;
    fd_cmd[6] = shift[minor];
    fd_cmd[7] = step[minor];
    fd_cmd[8] = f->precomp;
    
    while (ct < udata.u_nblock) {
        /* For each block we need to load we work out where to find it */
        fd_cmd[1] = udata.u_block / f->sectors;
        fd_cmd[2] = udata.u_block % f->sectors;
        fd_cmd[4] = ((uint16_t)udata.u_dptr) & 0xFF;
        fd_cmd[5] = ((uint16_t)udata.u_dptr) >> 8;
        /* Some single density media has sectors numbered from zero */
        if (!(f->config & FDC_SEC0))
            fd_cmd[2]++;
        /* Reading 40 track media on an 80 track drive */
        if (f->config & FDC_DSTEP)
            fd_cmd[1] <<= 1;
        /* Now try the I/O */
        for (tries = 0; tries < 4 ; tries++) {
            kprintf("doop\n");
            err = fd_operation(driveptr);
            kprintf("opdone %x\n", err);
            if (err == 0)
                break;
            if (!is_read && err != 0xFF && (err & 0x40)) {
                udata.u_error = EROFS;
                return -1;
            }
            /* Reposition the head */
            if (tries > 1)
                fd_restore(driveptr);
        }
        if (tries == 4)
            goto bad;
        udata.u_dptr += size[minor];
        udata.u_block++;
        ct++;
    }
    return udata.u_nblock << (7 + shift[minor]);
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
    uint16_t w;
    switch(request) {
        case FDIO_GETCAP:
            return uput(fdcap + minor, buffer, sizeof(struct fdcinfo));
        case FDIO_GETINFO:
            return uput(fdc + minor, buffer, sizeof(struct fdcinfo));
        case FDIO_SETINFO:
            /* Ick.. but we are not portable code so we know how it packs */
            if (uget(fdc + minor, buffer, 7))
                return -1;
            fdc[minor].features &= fdcap[minor].features;
            w = fdc[minor].features;
            s = fdc[minor].steprate;
            /* Check chip and clock details */
            /* WD1771 - 6 6 10 20 */
            if (s >= 20)
                s = 3;
            else if (s >= 10)
                s = 2;
            else
                s = 0;
            step[minor] = s;
            if (!(fdc[minor].config & FDC_PRECOMP))
                fdc[minor].precomp = 255;	/* Never precomp */
            switch(w &= FDF_SECSIZE) {
            case 128:
                s = 2;
                break;
            case 256:
                s = 1;
                break;
            default:
            case 512:
                s = 0;
                break;
            }
            shift[minor] = s;
            size[minor] = w;
            /* Force reconfiguration */
            fd_tab[minor] = 0xFF;
            fd_selected = 255;
            return 0;
        case FDIO_FMTTRK:
            return -1;
        case FDIO_RESTORE:
            fd_selected = 255;	/* Force a re-configure */
            return do_fd_restore(minor);
    }
    return -1;
}
