#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <tinydisk.h>
#include <devide_sunrise.h>
#include <msx.h>

uint16_t ide_error;
uint16_t ide_base = 0x7E00;
uint8_t *devide_buf;

/* Remember if either disk needs a cache flush */
static uint8_t sunrise_dirty[2];
static uint8_t sunrise_cache[2];

struct msx_map sunrise_u, sunrise_k;

static void delay(void)
{
    timer_t timeout;

    timeout = set_timer_ms(25);

    while(!timer_expired(timeout))
       plt_idle();
}

static int sunrise_xfer(uint_fast8_t drive, bool is_read, uint32_t lba, uint8_t *dptr)
{
    uint8_t mask = (drive & 1) ? 0xF0 : 0xE0;

    if (is_read == 0)
        sunrise_dirty[drive] = 1;
    disk_lba = lba;
    disk_dptr = dptr;
    disk_rw = is_read;
    if (blk_xfer_bounced(do_ide_xfer, mask) == 0) {
        if (ide_error == 0xFF)
            kprintf("ide%d: timeout.\n", drive);
        else
            kprintf("ide%d: status %x\n", drive, ide_error);
        return 0;
    }
    return 1;
}        
    
static int sunrise_ioctl(uint_fast8_t dev, uarg_t request, char *data)
{
    if (request != BLKFLSBUF)
        return -1;

    if (sunrise_dirty[dev] == 0 || sunrise_cache[dev] == 0)
        return 0;
    sunrise_dirty[dev] = 0;

    if (do_ide_flush_cache(dev ? 0xF0 : 0xE0)) {
        udata.u_error = EIO;
        return -1;
    }
    return 0;
}

static void sunrise_init_drive(uint8_t drive)
{
    uint8_t mask = drive ? 0xF0 : 0xE0;
    uint8_t *dptr;
    int dev;
    uint_fast8_t i;

    kprintf("%d: ", drive);
    devide_buf = tmpbuf();
    if (do_ide_init_drive(mask) == NULL)
    goto failout;
    /* Check the LBA bit is set, and print the name */
    dptr = devide_buf + 54;		/* Name info */
    if (*dptr)
        for (i = 0; i < 20; i++) {
	    kputchar(dptr[1]);
	    kputchar(*dptr);
	    dptr += 2;
	}
    if (!(devide_buf[99] & 0x02)) {
        kputs("- non LBA");
        goto failout;
    }
    dev = td_register(drive, sunrise_xfer, sunrise_ioctl, 1);
    if (dev < 0)
        goto failout2;

#if 0
    /* TODO: when tinyide gets there */
        blk->flush = sunrise_flush_cache;
#endif

    if( !(((uint16_t*)devide_buf)[82] == 0x0000 && ((uint16_t*)devide_buf)[83] == 0x0000) ||
         (((uint16_t*)devide_buf)[82] == 0xFFFF && ((uint16_t*)devide_buf)[83] == 0xFFFF) ){
	/* command set notification is supported */
	if(devide_buf[164] & 0x20) {
	    /* write cache is supported */
	    sunrise_cache[dev] = 1;
	}
    }

    /* done with our temporary memory */
    tmpfree(devide_buf);
    return;
failout:
    kputs("\n");
failout2:
    tmpfree(devide_buf);
}

static const uint16_t sunrise_roms[4] = {
    0x8FB3,
    0xBB0E,
    0xABF2,
    0x0000
};

void sunrise_probe(void)
{
    uint8_t slot, subslot;
    uint8_t i;

    i = device_find(sunrise_roms);
    if (i == 0xFF)
        return;

    /* Generate and cache the needed mapping table */
    memcpy(&sunrise_k, map_slot1_kernel(i), sizeof(sunrise_k));
    memcpy(&sunrise_u, map_slot1_user(i), sizeof(sunrise_u));
#ifdef DEBUG
    kprintf("sunrise_k: %2x %2x %2x %2x %2x %2x\n",
        sunrise_k.private[0], sunrise_k.private[1], sunrise_k.private[2],
        sunrise_k.private[3], sunrise_k.private[4], sunrise_k.private[5]);
    kprintf("sunrise_u: %2x %2x %2x %2x %2x %2x\n",
        sunrise_u.private[0], sunrise_u.private[1], sunrise_u.private[2],
        sunrise_u.private[3], sunrise_u.private[4], sunrise_u.private[5]);
#endif

    do_ide_begin_reset();
    delay();
    do_ide_end_reset();
    delay();
    for (i = 0; i < 2; i++)
        sunrise_init_drive(i);
}
