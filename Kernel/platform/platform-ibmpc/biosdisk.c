/*
 *	PC BIOS disk driver
 *
 *	FIXME: make this use the block stuff and partitions
 *	FIXME2: some crapware blows up if the buffer crosses a 64K boundary
 *	so we need to check this for user (and do smaller I/O) and it probably
 *	means we need to 512 byte align the buffer cache (ick). Whle user is
 *	4K aligned they could pass a pointer on a DMA boundary so we may need
 *	a global disk bounce buffer too 8(
 *	(OTOH it's a real hardware limit so we can't escape it in native
 *	drivers either)
 *	FIXME3: some crapware also blows up if a track or head boundary
 *	is crossed, maybe we should just do 512 bytes at a time and cry
 *	in our beer.
 *	FIXME4: for floppies we need to try and read sector 0/0/1 on open
 *	and use fn 17 to try different settings until it works (and work out
 *	how to deal with 1.44 ??)
 *	FIXME5: we need an ioctl to setup and use the format services for
 *	floppy
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devhd.h>
#include <bios.h>

#define MAX_HD	4

struct disk_geom {
    uint8_t secs;
    uint8_t heads;
    uint16_t cyls;
    uint8_t max;
    uint8_t dev;
};

static uint8_t floppies;
static uint8_t nfloppy;
static uint8_t nhd;

static struct disk_geom floppy[4];
static struct disk_geom harddisk[MAX_HD];

static int bios_transfer(bool is_read, uint8_t rawflag,
    struct disk_geom *g);

int fd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    return bios_transfer(true, rawflag, &floppy[minor]);
}

int fd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    return bios_transfer(false, rawflag, &floppy[minor]);
}

int hd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    return bios_transfer(true, rawflag, &harddisk[minor]);
}

int hd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    return bios_transfer(false, rawflag, &harddisk[minor]);
}

/* FIXME: udata.u_page points into the mm so we need a helper to get our
   mapping *but* we don't know if it's cs: or ds: because we might be
   loading it ???? */
static int bios_transfer(bool is_read, uint8_t rawflag, struct disk_geom *g)
{
    uint16_t dptr;
    uint16_t ct = 0;
    uint32_t st;
    uint16_t page = kernel_ds;
    uint16_t left;
    uint16_t cylsec;
    uint8_t tries;

    /* Sort the actual request out */
    if(rawflag == 1 || rawflag == 3) {
        if (d_blkoff(9))
            return -1;
        /* We need a standard way to wrap the segment gets and page so
           we can share code with PDP11 etc */
        if (rawflag == 1)
            page = get_data_segment(udata.u_page2);
        else
            page = get_code_segment(udata.u_page2);
#ifdef SWAPDEV
    } else if (rawflag == 2) {		/* Swap device special */
        page = swappage;		/* Acting on this page */
        /* FIXME: EMM means bank flipping here */
#endif
    }

    dptr = (uint16_t)udata.u_dptr;
    left = udata.u_nblock;

    /* Try and read as much as we can in one go */
    while (ct < udata.u_nblock) {
        uint16_t n = left;
        uint8_t sec = udata.u_block % g->secs + 1;
        uint16_t cyl = udata.u_block / g->secs;
        uint8_t head = cyl / g->cyls;
        cyl %= g->cyls;
        cylsec = (cyl << 8) | sec;
        cylsec |= (cyl & 0x300) >> 10;
        
        if (n > g->max)
            n = g->max;

        /* Try the I/O multiple times if it fails */
        for (tries = 0; tries < 3; tries ++) {
            if (is_read)
                st = bioshd_read(cylsec, g->dev, head, page, dptr, n);
            else
                st = bioshd_write(cylsec, g->dev, head, page, dptr, n);
            /* st will be 00:secs, or 00:random, depending upon the BIOS
               if we succeeded. If CF is set and it's unknown we'll return
               0xFFxxxx */
            if (st <= 0xFF)
                break;
            /* Try and do partial writes, also try and deal with any crap
               BIOS that can't handle multi-track */
            if (tries == 2) {
                bioshd_reset(g->dev);
                n = 1;
            }
        }
        if (tries == 3) {
            kprintf("bios disk %d: block %d, error %d\n", g->dev, udata.u_block, st);
            break;
        }
        /* Adjust all our status */
        udata.u_block += n;
        ct += n;
        left -= n;
        dptr +=  n << 9;
    }
    return ct << 9;
}

int fd_open(uint8_t minor, uint16_t flag)
{
    if (!(floppies  & (1 << minor))) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int hd_open(uint8_t minor, uint16_t flag)
{
    if (minor >= nhd) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

void biosdisk_init(void)
{
    struct disk_geom *p = harddisk;
    uint32_t bits;
    uint16_t dx,cx;
    int i;

    /* How many disks do we have and what sort ? */
    nfloppy = equipment_word & 0x01;
    if (nfloppy)
        nfloppy = 1 + (equipment_word >> 6) & 3;
    for (i = 0; i < nfloppy; i++) {
        bits = bioshd_param(i);
        if (bits == 0xFFFF)
            continue;
        dx = bits >> 16;
        cx = bits;
        floppy[i].heads = dx >> 8;
        floppy[i].cyls = cx >> 8;
        floppy[i].secs = cx & 0x3F;
        floppy[i].cyls |= ((cx & 0xC0) << 2);
        floppy[i].dev = i;
        /* Keep to sectors/track or less */
        floppy[i].max = cx & 0x3F;
    }
    /* Now the hard disks.. This is so much more fun. We need to be careful
       not to read further once we have all the disks we should as the BIOS
       may make up imaginary drives! */
    nhd = biosdata_read(0x75);	/* Because we can't trust anything else */
    for (i = 0; i < 0x7f && nhd; i++) {
        bits = bioshd_param(0x80 + i);
        if (bits == 0xFFFF)
            continue;
        nhd--;
        p->heads = dx >> 8;
        p->cyls = cx >> 8;
        p->secs = cx & 0x3F;
        p->cyls |= ((cx & 0xC0) << 2);
        /* Assume the BIOS can multitrack.. we may want to be cautious about
           this for old 8086 boxes */
        p->max = 255;
        p->dev = 0x80 + i;
        p++;
    }
    nhd = i;
}
