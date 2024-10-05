/* 
 * z80pack fd driver
 *
 * Hard disks I and J are 4MB (255 track 128 sector). P is 512MB using
 * 16384 spt. All sectors 128 bytes.
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devfd.h>
#include <fdc.h>

#define fd_drive	10
#define	fd_track	11
#define	fd_sectorl	12
#define fd_cmd		13
#define fd_status	14
#define	fd_dmal		15
#define fd_dmah		16
#define fd_sectorh	17

/* floppies. 26 128 byte sectors, not a nice way to use all of them in 512's */
static unsigned sectrack[16] = {
    26, 26, 26, 26,
    0, 0, 0, 0,
    128, 128, 0, 0,
    0, 0, 0, 16384
};

int fd_transfer(bool is_read, uint_fast8_t minor, uint_fast8_t rawflag);

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
    80,
    1,
    1,
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
    case FDIO_FMTTRK:
        /* Virtual fd has no format mechanism */
        return 0;
    case FDIO_RESTORE:
        /* Virtual restore is meaningless */
        return 0;
    case FDIO_SETSKEW:
        return uput(skewtab + minor, buffer, 32);
    default:
        return -1;
    }
}

/* We will wrap on big disks if we ever try and support the Z80Pack P:
   that wants different logic */
static void fd_geom(unsigned minor, uint32_t block)
{
    /* Turn block int track/sector 
       and write to the controller.
       Forced to do real / and % */
    /* To avoid expensive 32bit maths handle the big drive specially */
    unsigned track;
    unsigned sector;

    if (minor == 15) {	/* P drive */
        /* 16384 spt */
        sector = ((uint16_t)block) & 0x3FFF;
        track = block >> 14;
        sector++;
        out(fd_sectorl, sector & 0xFF);
        out(fd_sectorh, sector >> 8);
    } else if (minor >= 4) {
        /* Hard disk - 128 spt */
        sector = ((uint16_t)block) & 0x7F;
        track = ((uint16_t)block) >> 7;
        sector++;
        out(fd_sectorl, sector & 0xFF);
        out(fd_sectorh, 0);
    } else {
        /* Floppy */
        track = ((uint16_t)block) / sectrack[minor];
        sector = ((uint16_t)block) % sectrack[minor];
        out(fd_sectorl, skewtab[minor][sector]);
        out(fd_sectorh, 0);
    }
    out(fd_track, track);
}

int fd_transfer(bool is_read, uint_fast8_t minor, uint_fast8_t rawflag)
{
    register uint16_t dptr;
    uint16_t ct = 0;
    uint_fast8_t st;
    uint_fast8_t map = 0;
    uint16_t *page = &udata.u_page;
    uint32_t block;

    if(rawflag == 1) {
        if (d_blkoff(9))
            return -1;
        map = 1;
#ifdef SWAPDEV
    } else if (rawflag == 2) {		/* Swap device special */
        page = &swappage;		/* Acting on this page */
        map = 1;
        /*
         *	In the z80pack case this is simpler than usual. Be very
         *	careful how you implement the swap device. On most platforms
         *	we have user space in part of the "common" which means you
         *	must be prepared to switch common segment as well during
         *	a swap, or to perform mapping games using the banks
         */
#endif
    }

    udata.u_nblock *= (BLKSIZE / 128);
    block = udata.u_block << 2;
    /* Read the disk in four sector chunks. FIXME We ought to cache the geometry
       and just bump sector checking for a wrap. */

    /* FIXME: move core to using usize_t ? */
    dptr = (uint16_t)udata.u_dptr;
    while (ct < udata.u_nblock) {
        out(fd_drive, minor);
        fd_geom(minor, block);
        /* The Z80pack DMA uses the current MMU mappings... beware that
         * is odd - but most hardware would be PIO (inir/otir etc) anyway */
        out(fd_dmal, dptr & 0xFF);
        out(fd_dmah, dptr >> 8);

#ifdef CONFIG_SWAP_ONLY
        /* No banking problems in swap only mode */
        out(fd_cmd , 1 - is_read);
#else
        if (map == 0)
            out(fd_cmd, 1 - is_read);
        else	/* RAW I/O - switch to user bank and issue command via
                   a helper in common */
            fd_bankcmd(1 - is_read, page);
#endif

        st = in(fd_status);
        /* Real disks would need retries */
        if (st) {
            kprintf("fd%d: block %d, error %d\n", minor, udata.u_block, st);
            break;
        }
        block++;
        ct++;
        dptr += 128;
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
