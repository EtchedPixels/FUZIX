#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <blkdev.h>
#include <pcw8256.h>
#include <devvd.h>

struct cpm3_dpb {
    uint16_t	spt;	/* 128 byte records per track */
    uint8_t	bsh;	/* block shift */
    uint8_t	blm;	/* block mask */
    uint8_t	exm;	/* extent mask */
    uint16_t	dsm;	/* blocks on disk - 1 */
    uint16_t	drm;	/* directory entries */
    uint8_t	al0,al1;/* allocation map */
    uint16_t	cks;	/* checksum vector */
    uint16_t	off;	/* offset for reserved tracks */
    uint8_t	psh;	/* physical sector shift */
    uint8_t	psm;	/* physical sector mask */
};

#define MAXDRIVE	4
#define VD_DRIVE_NR_MASK	0xFF


static struct cpm3_dpb dpb[MAXDRIVE];
struct cpm3_dpb *vd_dpb;
uint16_t vd_track;
uint16_t vd_sector;
uint16_t vd_drive_op;
uint8_t vd_mapping;
uint8_t vd_page;

static uint8_t devvd_transfer_sector(void)
{
    uint_fast8_t drive = blk_op.blkdev->driver_data & VD_DRIVE_NR_MASK;
    uint16_t lba = blk_op.lba;	/* We know it'll fit 16bit */

    /* Fixme: cache the logical spt */
    vd_track = lba / (dpb[drive].spt >> 2);
    vd_sector = lba % (dpb[drive].spt >> 2);

    vd_mapping = blk_op.is_user;
    vd_page = blk_op.swap_page;
    vd_drive_op = (drive << 8) | (blk_op.is_read ? 0x04 : 0x05); 
    vd_dpb = dpb + drive;
    
    if (vd_do_op(blk_op.addr))
        return 0;
    return 1;
}
    
static int devvd_flush_cache(void)
{
    return 0;
}

static uint8_t devvd_open(uint8_t i)
{
    dpb[i].spt = i;	/* cheap hack for the asm interface */
    return vd_do_open(&dpb[i]);
}

void devvd_probe(void)
{
    blkdev_t *blk;
    uint8_t i;

    for (i = 2; i < MAXDRIVE; i++) {
        if (devvd_open(i) == 0) {
            /* Check the DPB is ok */
            if (dpb[i].spt & 3) {
                kprintf("vd%d: sectors/track unsuitable.\n");
                continue;
            }
            blk = blkdev_alloc();
            if (blk == NULL) {
                kputs("vd: too many devices.\n");
                return;
            }
            blk->transfer = devvd_transfer_sector;
            blk->flush = devvd_flush_cache;
            blk->driver_data = i & VD_DRIVE_NR_MASK;
            /* Assumes 512 byte sectors */
            blk->drive_lba_count = (dpb[i].dsm + 1) << (dpb[i].bsh - 2);
            blkdev_scan(blk, SWAPSCAN);
        }
    }
}
