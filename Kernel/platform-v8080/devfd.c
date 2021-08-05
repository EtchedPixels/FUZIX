/* 
 * z80pack fd driver
 *
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <fdc.h>
#include <devfd.h>

/* floppies. 26 128 byte sectors, not a nice way to use all of them in 512's */
static int sectrack[16] = {
    26, 26, 26, 26,
    0, 0, 0, 0,
    128, 128, 0, 0,
    0, 0, 0, 0
};

static int fd_transfer(bool is_read, uint_fast8_t minor, uint_fast8_t rawflag);

int fd_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
    flag;
    return fd_transfer(true, minor, rawflag);
}

int fd_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
    flag;
    return fd_transfer(false, minor, rawflag);
}

int hd_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
    flag;
    return fd_transfer(true, minor + 8, rawflag);
}

int hd_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
    flag;
    return fd_transfer(false, minor + 8, rawflag);
}

/* Floppies are fixed to classic IBM 8" SD format */
static struct fdcinfo fdcap = {
    0,
    FDTYPE_8SSSD,
    0,
    FDF_SD|FDF_8INCH|FDF_SEC128,
    26,			/* 26 sectors per track */
    77,
    1,
    1
};

static uint8_t skewtab[4][32] = {
    { 1,7,13,19,25,5,11,17,23,3,9,15,21,2,8,14,20,26,6,12,18,24,4,10,16,22, },
    { 1,7,13,19,25,5,11,17,23,3,9,15,21,2,8,14,20,26,6,12,18,24,4,10,16,22, },
    { 1,7,13,19,25,5,11,17,23,3,9,15,21,2,8,14,20,26,6,12,18,24,4,10,16,22, },
    { 1,7,13,19,25,5,11,17,23,3,9,15,21,2,8,14,20,26,6,12,18,24,4,10,16,22, }
};

int fd_ioctl(uint_fast8_t minor, uarg_t request, char *buffer)
{
    switch(request) {
    case FDIO_GETCAP:
        return uput(&fdcap, buffer, sizeof(struct fdcinfo));
    case FDIO_SETSKEW:
        return uput(skewtab + minor, buffer, 32);
    default:
        return -1;
    }
}

/* We will wrap on big disks if we ever try and support the Z80Pack P:
   that wants different logic */
static void fd_geom(int minor, blkno_t block)
{
    /* Turn block int track/sector 
       and write to the controller.
       Forced to do real / and % */
    int track = block / sectrack[minor];
    int sector = block % sectrack[minor];
    if (minor >= 4) {
        /* Hard disk */
        sector++;
        fd_sector = sector;
    } else {
        /* Floppy */
        fd_sector = skewtab[minor][sector];
    }
    fd_track = track;
}

static int fd_transfer(bool is_read, uint_fast8_t minor, uint_fast8_t rawflag)
{
    uint16_t dptr;
    uint16_t ct = 0;
    uint_fast8_t st;

    fd_page = 0;
    if(rawflag == 1) {
        if (d_blkoff(7))
            return -1;
        fd_page = udata.u_page;

#ifdef SWAPDEV
    } else if (rawflag == 2) {		/* Swap device special */
        fd_page = swappage;		/* Acting on this page */
        /*
         *	In the z80pack case this is simpler than usual. Be very
         *	careful how you implement the swap device. On most platforms
         *	we have user space in part of the "common" which means you
         *	must be prepared to switch common segment as well during
         *	a swap, or to perform mapping games using the banks
         */
        udata.u_nblock *= (BLKSIZE / 128);
        /* Limits us to 2^14 blocks (16MB) */
        udata.u_block <<= 2;
#endif
    } else { /* rawflag == 0 */
        udata.u_nblock = BLKSIZE / 128;		/* BLKSIZE bytes implied */
        /* Limits us to 2^14 blocks (16MB) */
        udata.u_block <<= 2;
    }
    /* Read the disk in four sector chunks. FIXME We ought to cache the geometry
       and just bump sector checking for a wrap. */

    fd_dma = (uint16_t)udata.u_dptr;
    while (ct < udata.u_nblock) {
        fd_drive = minor;
        fd_geom(minor, udata.u_block);
        /* The Z80pack DMA uses the current MMU mappings... beware that
         * is odd - but most hardware would be PIO (inir/otir etc) anyway */
        fd_cmd = 1 - is_read;

        st = fd_op();
        /* Real disks would need retries */
        if (st) {
            kprintf("fd%d: block %d, error %d\n", minor, udata.u_block, st);
            break;
        }
        udata.u_block++;
        ct++;
        fd_dma += 128;
    }
    return ct << 7;
}

/* Media is fixed at start up time */
int fd_open(uint_fast8_t minor, uint16_t flag)
{
    flag;
    if(minor >= 8 || !sectrack[minor]) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int hd_open(uint_fast8_t minor, uint16_t flag)
{
    flag;
    if(minor >= 8 || !sectrack[minor + 8]) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}
