/* 
 * PDP/11 RK05 cartridge driver
 * We assume RK05 packs for PDP/11 the moment (2/202/12 geometry)
 *
 * Also assumed is one unit. But that's easily fixed
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <dev_rk.h>

static uint16_t *rkds = (uint16_t *)0177400;
static uint16_t *rker = (uint16_t *)0177402;
static uint16_t *rkcs = (uint16_t *)0177404;
static uint16_t *rkwc = (uint16_t *)0177406;
static uint16_t *rkba = (uint16_t *)0177410;
static uint16_t *rkda = (uint16_t *)0177412;

static int rk_probe(uint8_t minor)
{
    /* FIXME: timeout ?? */
    while(!(*rkcs & 0x80));
    *rkda = minor << 12;
    *rkcs = 0x0B;	/* Drive reset, go */
    while(!(*rkcs & 0x80));
    /* Now check the drive status */
    if (!(*rkds & 0x80))
        return 0;
    if (*rkds & 0x20)
        return 3;
    return 1;
}
    
static int rk_transfer(bool is_read, uint8_t minor, uint8_t rawflag)
{
    uint16_t ct = 0;
    uint16_t st;
    uint16_t *page = NULL;
    uint8_t cmd = is_read ?  4 : 2;

    if (rk_offline & (1 << minor)) {
        udata.u_error = EIO;
        return -1;
    }
    if(rawflag == 1) {
        if (d_blkoff(9))
            return -1;
        page =  &udata.u_page;
#ifdef SWAPDEV
    } else if (rawflag == 2) {		/* Swap device special */
        page = &swappage;		/* Acting on this page */
#endif
    } else { /* rawflag == 0 */
    }


    /* We don't try and merge linearly sequential writes but these are so rare
       (mkfs basically) even without vm.. We may want to reconsider for swap
       however */
    /* Overlapped seek is also supported but we can't really use it */

    while (ct < udata.u_nblock) {
        /* It's not quite as simple as it seems because while there is a single
           disk address register it's up to use to set the bits properly for
           sectors, suface, cylinder, drive */
        uint16_t darv = minor << 13;
        uint8_t sector;
        uint8_t surface = 0x00;
        uint16_t cylinder;

        /* Assume a PDP/11 catridge not a PDP/8 one */
        sector = udata.u_block % 12;
        cylinder = udata.u_block /12;
        /* Check this logic: there are spare tracks but are they hard/soft
           spared ? */
        if (cylinder > 202) {
            cylinder -= 202;
            surface = 0x10;
        }
        darv |= sector | surface
        darv |= (cylinder << 5);

        /* Wait until ready */        
        while(!(*rkcd & 0x80));

        /* Load the target, addresses and command */
        *rkda = darv;
        *rkwc = -256;	/* 512 bytes for now */
        *rkba = udata.u_dptr;
        *rkcs = cmd | 1;	/* Needs top bits of VA when we do virtual */

        while(!(*rkcd & 0x80));

        st = *rker;
        if (st) {
            kprintf("fd%d: block %d, error %x/%x\n", minor, udata.u_block, st,
                *rkds);
            if (st & 0xFFF0)
                rk_offline |= (1 << minor);
            break;
        }
        udata.u_block++;
        ct++;
        udata.u_dptr += 512;
    }
    return ct << 9;
}

/* For now assume a single controller, max 8 packs, no partitioning support */

int rk_open(uint8_t minor, uint16_t flag)
{
    flag;
    uint8_t r;
    /* No such drive or drive not loaded */
    if(minor >= 8 || !(r = rk_probe(minor))) {
        udata.u_error = ENODEV;
        return -1;
    }
    /* Read only media */
    if ((r & 2) && O_ACCMODE(flag)) {
        udata.u_error = EROFS;
        return -1;
    }
    return 0;
}

int rk_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    return rk_transfer(true, minor + 8, rawflag);
}

int rk_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    return rk_transfer(false, minor + 8, rawflag);
}
