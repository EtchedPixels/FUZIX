/* 
 * z80pack fd driver
 *
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devfd.h>

__sfr __at 10 fd_drive;
__sfr __at 11 fd_track;
__sfr __at 12 fd_sectorl;
__sfr __at 13 fd_cmd;
__sfr __at 14 fd_status;
__sfr __at 15 fd_dmal;
__sfr __at 16 fd_dmah;
__sfr __at 17 fd_sectorh;

/* floppies. 26 128 byte sectors, not a nice way to use all of them in 512's */
static int sectrack[16] = {
    26, 26, 26, 26,
    0, 0, 0, 0,
    128, 128, 0, 0,
    0, 0, 0, 0
};

static int fd_transfer(bool is_read, uint8_t minor, uint8_t rawflag);

int fd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    return fd_transfer(true, minor, rawflag);
}

int fd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    return fd_transfer(false, minor, rawflag);
}

int hd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    return fd_transfer(true, minor + 8, rawflag);
}

int hd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    return fd_transfer(false, minor + 8, rawflag);
}

/* We will wrap on big disks if we ever try and support the Z80Pack P:
   that wants different logic */
static void fd_geom(int minor, blkno_t block)
{
    /* Turn block int track/sector 
       and write to the controller.
       Forced to do real / and % */
    int track = block / sectrack[minor];
    int sector = block % sectrack[minor] + 1;
    fd_sectorl = sector & 0xFF;
    fd_sectorh = sector >> 8;
    fd_track = track;
}

static int fd_transfer(bool is_read, uint8_t minor, uint8_t rawflag)
{
    uint16_t dptr;
    uint16_t ct = 0;
    uint8_t st;
    int map = 0;
    uint16_t *page = &udata.u_page;

    if(rawflag == 1) {
        if (d_blkoff(7))
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

    /* FIXME: move core to using usize_t ? */
    dptr = (uint16_t)udata.u_dptr;
    while (ct < udata.u_nblock) {
        fd_drive = minor;
        fd_geom(minor, udata.u_block);
        /* The Z80pack DMA uses the current MMU mappings... beware that
         * is odd - but most hardware would be PIO (inir/otir etc) anyway */
        fd_dmal = dptr & 0xFF;
        fd_dmah = dptr >> 8;

#ifdef CONFIG_SWAP_ONLY
        /* No banking problems in swap only mode */
        fd_cmd = 1 - is_read;
#else
        if (map == 0)
            fd_cmd = 1 - is_read;
        else	/* RAW I/O - switch to user bank and issue command via
                   a helper in common */
            fd_bankcmd(1 - is_read, page);
#endif

        st = fd_status;
        /* Real disks would need retries */
        if (st) {
            kprintf("fd%d: block %d, error %d\n", minor, st, udata.u_block);
            break;
        }
        udata.u_block++;
        ct++;
        dptr += 128;
    }
    return ct << 7;
}

int fd_open(uint8_t minor, uint16_t flag)
{
    flag;
    if(minor >= 8 || !sectrack[minor]) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int hd_open(uint8_t minor, uint16_t flag)
{
    flag;
    if(minor >= 8 || !sectrack[minor + 8]) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}
