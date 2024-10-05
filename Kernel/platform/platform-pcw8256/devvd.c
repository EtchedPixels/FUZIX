#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <tinydisk.h>
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

static int devvd_xfer(uint_fast8_t drive, bool is_read, uint32_t lba, uint8_t *dptr)
{
    /* Fixme: cache the logical spt */
    vd_track = lba / (dpb[drive].spt >> 2);
    vd_sector = lba % (dpb[drive].spt >> 2);

    vd_drive_op = (drive << 8) | (is_read ? 0x04 : 0x05);
    vd_dpb = dpb + drive;

    if (vd_do_op(dptr))
        return 0;
    return 1;
}
    
static uint8_t devvd_open(uint8_t i)
{
    dpb[i].spt = i;	/* cheap hack for the asm interface */
    return vd_do_open(&dpb[i]);
}

void devvd_probe(void)
{
    uint8_t i;

    for (i = 2; i < MAXDRIVE; i++) {
        if (devvd_open(i) == 0) {
            /* Check the DPB is ok */
            if (dpb[i].spt & 3) {
                kprintf("vd%d: sectors/track unsuitable.\n");
                continue;
            }
            kprintf("%c: ", i + 'A');
            if (td_register(i, devvd_xfer, td_ioctl_none, 1) < 0)
                continue;
        }
    }
}
