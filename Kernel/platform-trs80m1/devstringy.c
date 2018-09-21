#include <kernel.h>
#include <version.h>
#include <tape.h>
#include <kdata.h>
#include <devlpr.h>
#include <printf.h>
#include <trs80.h>
#include <devstringy.h> 

/*
 *	Stringy tape wrapper. Note that the asm code uses the ROM which
 *	also uses some values in 0x40xx (40B1 and 401A in particular)
 */
static uint8_t fileid = 1;
static uint8_t mode;
static uint8_t inpos = 0;		/* in position */
static uint8_t inio = 0;
static uint8_t curtape = 255;
static int busy = 0;
uint8_t tape_err;

static int tape_error(void)
{
    uint8_t a = tape_err;
    tape_err = 0;

    /* EOF on read */
    if (a & 0x80)
        return 0;
    /* Write protected */
    if (a & 0x01)
        udata.u_error = EROFS;
    /* BREAK */        
    if (a & 0x02)
        udata.u_error = EINTR;
    /* End of tape while writing */
    else if (a & 0x04)
        udata.u_error = ENOSPC;
    /* Buffer too small (read) */
    else if (a & 0x20)
        udata.u_error = EINVAL;	/* Not a good error for this really */
    else
        udata.u_error = EIO;
    kprintf("tape: error %x\n", a);
    inpos = inio = 0;
    return -1;
}

static int tape_rewind(void)
{
    if (!tape_op(0, TAPE_REWIND)) {
    	/* rewind */
    	fileid = 1;
        inpos = 0;
        return 0;
    }
    return tape_error();
}

int tape_open(uint8_t minor, uint16_t flag)
{
    minor; flag;
    uint8_t unit;

    /* Check for the floppy tape ROM */
    if (trs80_model == TRS80_MODEL3 || *((uint16_t *)0x3034) != 0x3C3C) {
        udata.u_error = ENODEV;
        return -1;
    }
    if (busy) {
        udata.u_error = EBUSY;
        return -1;
    }

    unit = minor  & 7;

    if (unit != curtape && tape_op(unit, TAPE_SELECT)) {
        udata.u_error = ENODEV;
        return -1;
    }

    /* Can't open for mixed read/write at the same time */
    if (O_ACCMODE(flag) == O_RDWR) {
        udata.u_error = EINVAL;
        return -1;
    }
    mode = O_ACCMODE(flag);

    if (minor & 0x08)
        if (tape_rewind())
            return -1;
    /* Only one drive can be used at a time */
    busy = 1;
    if (unit != curtape) {
        inio = 0;
        tape_err = 0;
        fileid = 1;
        curtape = unit;
    }
    return 0;
}

int tape_close(uint8_t minor)
{
    minor;
    busy = 0;
    inio = 0;
    inpos = 0;
    if (mode == O_WRONLY)
        if (tape_op(fileid, TAPE_CLOSEW))
            return tape_error();
    if (fileid < 99)
        fileid++;
    return 0;
}

static int tape_rw(uint8_t op)
{
    uint8_t pos = fileid;

    if (!inpos) {
            if (tape_op(pos, TAPE_FIND) == 0)
                inpos = 1;
            else
                return tape_error();
    }
    inio = 1;
    udata.u_done = tape_op(fileid, op);
    if (tape_err)
        return tape_error();
    return udata.u_done;
}

int tape_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    used(minor);
    used(rawflag);
    used(flag);
    return tape_rw(TAPE_READ);
}

int tape_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    used(minor);
    used(rawflag);
    used(flag);
    return tape_rw(TAPE_WRITE);
}

static struct mtstatus tapei = {
    MT_TYPE_EXATRON,
    0,
    ~0UL
};

int tape_ioctl(uint8_t minor, uarg_t op, char *ptr)
{
    used(minor);

    switch(op) {
        case MTSTATUS:
            if (inpos)
                tapei.mt_file = fileid;
            else
                tapei.mt_file = 0xFFFF;
            return uput(&tapei, ptr, sizeof(tapei));
    }
    /* Now calls we can only make when not mid stream */
    if (mode || inio)
        goto bad;

    switch (op) {
    case MTREWIND:
        return tape_rewind();
    case MTSEEKF:
        if (fileid > 99)
            goto bad;
        fileid++;
        if (tape_op(fileid, TAPE_FIND))
            goto bad;
        inpos = 1;
        return 0;
    case MTSEEKB:
        if (fileid < 2)
            goto bad;
        fileid--;
        if (tape_op(fileid, TAPE_FIND))
            goto bad;
        inpos = 1;
        return 0;
    case MTERASE:
        /* Erase from this point to the end of tape */
        if (fileid > 99)
            goto bad;
        if (tape_op(fileid, TAPE_ERASE))
            goto bad;
        inpos = 0;
        return 0;
    default:
        return -1;
    }
bad:
    udata.u_error = EINVAL;
    return -1;
}

void tape_init(void)
{
    *(uint16_t*)0x40b1 = 0x4000;	/* Somewhere to put the tape variables */
}
