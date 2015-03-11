#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devfd.h>
#include <sysconfig.h>

#define MAX_FD 2

extern uint8_t fdc_user;
extern uint16_t fdc_addr;
extern uint8_t rwcmd[9];
extern uint8_t seekcmd[3];

extern void fdc_motoron(void);
extern void fdc_motoroff(void);
extern uint8_t fdc_read(void);
extern uint8_t fdc_write(void);
extern uint8_t fdc_recal(void);
extern uint8_t fdc_seek(void);

uint8_t track[MAX_FD] = { 0xFF, 0xFF };

static int fd_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
    blkno_t block;
    uint16_t dptr;
    int ct = 0;
    int tries;
    uint8_t err = 0;
    uint8_t nblock;

    if(rawflag == 2)
        goto bad2;

    if (rawflag == 0) {
        dptr = (uint16_t)udata.u_buf->bf_data;
        block = udata.u_buf->bf_blk;
        nblock = 1;
    } else {
        if (((uint16_t)udata.u_offset|udata.u_count) & BLKMASK)
            goto bad2;
        dptr = (uint16_t)udata.u_base;
        block = udata.u_offset >> 9;
        nblock = udata.u_count >> 9;
    }
    
    rwcmd[1] = 1 << minor;
    rwcmd[2] = block / 9;
    rwcmd[3] = 0;		/* Single sided only for now */
    rwcmd[4] = 1 + block % 9;	/* Sector */
    rwcmd[6] = rwcmd[4];

    fdc_user = rawflag;

    while (ct < nblock) {
        fdc_addr = dptr;
        if (track[minor] != rwcmd[2]) {
            seekcmd[1] = 1 << minor;
            seekcmd[2] = rwcmd[2];
            fdc_seek();		/* FIXME: error handling */
        }
        for (tries = 0; tries < 4 ; tries++) {
            /* FIXME: need to return status properly and mask it */
            if (is_read)
                err = fdc_read();
            else
                err = fdc_write();
            if (err == 0)
                break;
        }
        if (tries > 1)		/* FIXME: set drive */
            fdc_recal();
        if (tries == 4)
            goto bad;

        dptr += 0x200;	/* Move on 512 bytes in the buffer */

        rwcmd[4]++;
        /* Step a track */
        if (rwcmd[4] > 10) {
            rwcmd[4] = 1;
            rwcmd[2]++;
        }
        rwcmd[6] = rwcmd[4];
        ct++;
    }
    return 1;
bad:
    kprintf("fd%d: error %x\n", minor, err);
bad2:
    udata.u_error = EIO;
    return -1;
}

int fd_open(uint8_t minor, uint16_t flag)
{
    flag;
    /* Check we have a floppy */
    if(!(sysconfig & CONF_PLUS3) || minor >= MAX_FD) {
        udata.u_error = ENODEV;
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
    flag;
    return fd_transfer(minor, false, rawflag);
}
