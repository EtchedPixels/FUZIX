#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devfd.h>
#include <devbeta.h>

#define MAX_FD	4

static uint8_t lastdisc = 0xFF;

/*
 *	First cut at a beta driver. This uses the ROM entry points as the
 *	beta has a design where you can't access the disk except with the
 *	ROM paged in.
 *
 *	We treat the ROM as a set of pieces to build a ROP type exploit.
 *
 *	Note: the ROM has 39xx all set to 0xFF which means that we can
 *	continue to take interrupts with this ROM paged in as we'll read
 *	0xFFFF and then take the vector as JR xx (where 0000 is still 0xF3
 *	as this rom also starts with a DI).
 */

static int beta_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
    blkno_t block;
    uint16_t ret;

    if(rawflag == 2)
        goto bad2;

    /* Select the right disc */

    if (lastdisc != minor) {
        betadev = minor;
        if (trdos_init())
            goto bad2;
        lastdisc = minor;
    }

    /* 16 sectors/track, 40 or 80 tracks but the interface expects logical
       tracks 0-n alternating sides. 256 bytes/sector always */

    if (rawflag == 0) {
        betaaddr = (uint16_t)udata.u_buf->bf_data;
        block = udata.u_buf->bf_blk;
        betacount = 2;
    } else {
        if ((udata.u_offset|udata.u_count) & 0x1FF)
            goto bad2;
        betaaddr = (uint16_t)udata.u_base;
        block = udata.u_offset >> 9;
        betacount = udata.u_count >> 8;
    }
    betasector = ((block >> 5) & 0x0F) + 1;
    betatrack = block >> 5;
    betauser = rawflag;
    betadev = minor;
    di();
    if (is_read)
        ret = trdos_read();
    else
        ret = trdos_write();
    ei();
    if (ret == 0)
        return betacount >> 1;
    kprintf("bfd%d: error %d\n", ret);
bad2:
    udata.u_error = EIO;
    return -1;
}

/* FIXME: how do we detect beta is present sanely ? */
int beta_open(uint8_t minor, uint16_t flag)
{
    int ret;

    flag;

    if(minor >= MAX_FD) {
        udata.u_error = ENODEV;
        return -1;
    }
    betadev = minor;

    /* Stop the TR-DOS code trying to make workspaces and crap into
       basic by putting its 'initialized' token into place */
    *(uint8_t *)0x5CB6 = 0xF4;
    *(uint8_t *)0x5D16 = 0x00;	/*FIXME: syscfg bits */

    ret = trdos_init();

    if (ret) {
        udata.u_error = EIO;
        lastdisc = 0xFF;
        return -1;
    }
    lastdisc = minor;
    return 0;
}

int beta_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    return beta_transfer(minor, true, rawflag);
}

int beta_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    return beta_transfer(minor, false, rawflag);
}
