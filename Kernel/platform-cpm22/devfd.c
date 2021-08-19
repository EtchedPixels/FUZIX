/* 
 * CP/M 2 FD/HD driver
 *
 * We map the Fuzix I/O onto CP/M volumes. It's really up to the BIOS how it
 * maps them and if for example it maps them as real raw media or not.
 *
 * TODO: removable media handling.
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devfd.h>
#include <cpm.h>
#include <sysmod.h>

static int fd_transfer(bool is_read, uint8_t minor, uint8_t rawflag);

static uint8_t cpm_sys;

static uint8_t cpm_drive = 0xFF;

static uint8_t fd_map[16], hd_map[16];
static uint8_t num_hd, num_fd;
static uint32_t secsize[16];

int fd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    cpm_sys = minor & 0x10;
    cpm_drive = fd_map[minor & 0x0F];
    return fd_transfer(true, 'f', rawflag);
}

int fd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    cpm_sys = minor & 0x10;
    cpm_drive = fd_map[minor & 0x0F];
    return fd_transfer(false, 'f', rawflag);
}

int hd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    cpm_sys = minor & 0x10;
    cpm_drive = hd_map[minor & 0x0F];
    return fd_transfer(true, 'h', rawflag);
}

int hd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    cpm_sys = minor & 0x10;
    cpm_drive = hd_map[minor & 0x0F];
    return fd_transfer(false, 'h', rawflag);
}

static struct cpm_dph *cpm_dph;
static struct cpm_dpb *cpm_dpb;
static uint8_t last_dev = 255;

static void cpm_geom(void)
{
    uint16_t track;
    uint16_t sec = udata.u_block % cpm_dpb->spt;
    /* Zero based sector */
    sec = cpm_sectran(sec, cpm_dph->xlt);
    cpm_setsec(sec);
    /* Logical track number */
    track = udata.u_block / cpm_dpb->spt;
    /* Avoid system area */
    if (!cpm_sys)
        track += cpm_dpb->off;
    cpm_settrk(track);
}

/*
 *	Switch drive
 */
static uint8_t cpm_setup_drive(void)
{
    if (cpm_drive != last_dev) {
        cpm_dph = cpm_seldsk(cpm_drive);
        if (cpm_dph == NULL) {
            kprintf("Unable to set up drive %c\n", 'A' + cpm_drive);
            udata.u_error = ENXIO;
            return 1;
        }
        last_dev = cpm_drive;
        cpm_dpb = cpm_dph->dpb;
    }
    return 0;
}

/* CP/M has no notion of sides - that is handled in the driver. Each
   volume is logically single sided. */

static int fd_transfer(bool is_read, uint8_t type, uint8_t rawflag)
{
    uint16_t ct = 0;
    uint8_t err;
    uint8_t hint;
    uint8_t bounce = 0;
    irqflags_t irq;

    if(rawflag == 1) {
        if (d_blkoff(7))
            return -1;
        cpm_map = udata.u_page;
#ifdef SWAPDEV
    } else if (rawflag == 2) {		/* Swap device special */
        cpm_map = swappage;
        udata.u_nblock *= (BLKSIZE / 128);
        udata.u_block <<= 2;
#endif
    } else { /* rawflag == 0 */
        udata.u_nblock = BLKSIZE / 128;		/* BLKSIZE bytes implied */
        udata.u_block <<= 2;
        cpm_map = 0;
    }

    if (type == 'f' && info->features & FEATURE_DMAFD)
        bounce = 1;
    if (type == 'h' && info->features & FEATURE_DMAHD)
        bounce = 1;

    cpm_busy = 1;

    /* Select the drive and check all is good */
    if (cpm_setup_drive()) {
        cpm_busy = 0;
        return -1;
    }

    /* Most Fuzix drivers don't worry about this and rely on the fs, but
       in CP/M or MP/M it's common to use drive letters as partitions and
       we therefore want to be fussier about normal I/O accesses. We allow
       overrruns on sys accesses as you should know what you are doing then
       and might need a way to access data for special cases */

    if (!cpm_sys && udata.u_block + udata.u_nblock >= secsize[cpm_drive]) {
        udata.u_error = EIO;
        cpm_busy = 0;
        return -1;
    }

    /* Use the scratch area the BIOS itself provides to CP/M when
       bouncing buffers */
    if (bounce)
        cpm_setdma(cpm_dph->buffer);

    ct = 0;
    while (ct < udata.u_nblock) {
        cpm_geom();

        if (bounce == 0)
            cpm_setdma(udata.u_dptr);

        if (info->features & FEATURE_IODI)
            irq = di();
        if (is_read) {
            err = cpm_diskread();
            if (bounce == 1)
               uput(cpm_dph->buffer, udata.u_dptr, 128);
        } else {
            /* For each 512 byte sector we use the pattern 2, 0, 0, 1
               write without pre-read, write defer, write defer, write */
            if ((udata.u_block & 3) == 0)
                hint = 2;
            else if ((udata.u_block & 3) != 3)
                hint = 0;	
            else
                hint = 1;
            if (bounce == 1)
                uget(udata.u_dptr, cpm_dph->buffer, 128);
            err = cpm_diskwrite(hint);
        }
        if (info->features & FEATURE_IODI)
            irqrestore(irq);

        if (err) {
            kprintf("%c: block %d, error %d\n",
                cpm_drive+'A', udata.u_block, err);
            break;
        }
        udata.u_dptr += 128;
        ct++;
        udata.u_block++;
    }
    cpm_busy = 0;
    return ct << 7;
}

int fd_open(uint8_t minor, uint16_t flag)
{
    flag;
    if((minor & 0x0F) >= num_fd) {
        udata.u_error = ENODEV;
        return -1;
    }
    cpm_drive = fd_map[minor & 0x0F];
    return cpm_setup_drive();
}

int hd_open(uint8_t minor, uint16_t flag)
{
    flag;
    if ((minor & 0x0F) >= num_hd ) {
        udata.u_error = ENODEV;
        return -1;
    }
    cpm_drive = hd_map[minor & 0x0F];
    return cpm_setup_drive();
}

int fd_close(uint8_t minor)
{
    return 0;
}

int hd_close(uint8_t minor)
{
    return 0;
}

/* Scan each CP/M disk and see which of the 16 drives are present. For each
   drive then look at the returned dpb and guess if it is hard or soft. */
void fdhd_init(void)
{
    int i;
    struct cpm_dph *h;
    struct cpm_dpb *d;
    uint16_t hdmask = 0;

    cpm_busy++;

    for (i = 0; i < 16; i++) {
        h = cpm_seldsk(i);
        if (h) {
            d = h->dpb;
            kprintf("%c: mapped to /dev/", i + 'A');
            secsize[i] = ((uint32_t)d->dsm + 1) << d->bsh;
            /* Fixed devices and anything >= 2MB we label as hard disk */
            if (d->cks == 0x8000 || secsize[i] >= 16384) {
                /* Probably hard disc */
                kprintf("hd%d\n", num_hd);
                hd_map[num_hd++] = i;
                if (i == info->swap)
                    found_swap(i, secsize[i]);
            } else {
                /* Probably floppy */
                kprintf("fd%d\n", num_fd);
                fd_map[num_fd++] = i;
            }
        }
    }
    cpm_busy--;    
}
