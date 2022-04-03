

/* These are only valid with the controlled space mapped at 0x4000, we need
   to rethink to avoid bounce buffers and crap */

#define	sm9234_wd	((volatile uint8_t *)0x4FD2
#define sm9234_wc	((volatile uint8_t *)0x4FD6
#define sm9234_rd	((volatile uint8_t *)0x4FD0
#define sm9234_rs	((volatile uint8_t *)0x4FD0

/*
 *	We have an 8K or 32K cache that we should be using better once this
 *	works. It meeans we can keep a pending write array. Whilst we can
 *	add a pending read model here the core OS can't yet really use it.
 *	We can also track what sector pairs are in our cache
 *
 *	Other considerations: no support for physical 256 byte I/O - do we care
 */
static void hfdc_sync(unsigned drive)
{
    if (!hfdc_busy)
        return;
    /* TIMEOUT and reset ? */
    while((sm9234_rs & 0x20) == 0);
    if (sm9234_rs & 0x0C)
        hfdc_error(drive);		/* Delayed write error */
    hfdc_busy = 0;
}

/*
 *	Reset the controller
 */

static void hfdc_reset(void)
{
    *sm9234_wc = 0x00;
    hfdc_busy = 1;
    hfdc_sync();
}

static void hfdc_error(unsigned drive)
{
    unsigned int isr, csr, dsr;

    *sm9234_wc = 0x47;

    isr = *sm9234_rd;
    csr = *sm9234_rd;
    dsr = *sm9234_rd;
    kprintf("hfdc %d: error %2x %2x %2x\n",
        drive, isr, csr, dsr);
}    

static void hfdc_recover(unsigned drive)
{
    *sm9234_wc = 0x03;
    hfdc_busy = 1;
    hfdc_sync();
    if (hfdc_rs & 0x0C)
        hfdc_error(drive);
}

/* Hard disk mode - will deal with floppy later */
void hfdc_transfer_sector(void)
{
    uint_fast8_t drive = blk_op.driver_data & HFDC_DRIVE_NR_MASK;
    uint16_t c, h, s;
    unsigned int retry = 0;

    /* Sync the controller */
    hfdc_sync(drive);
    hfdc_hd_select(drive);

    /* User data to the cache */
    if (blk_op.is_read == 0)
        hfdc_data_to_cache();

    /* Convert to CHS */
    s = blk_op.lba % spt[drive];
    h = s / heads[drive];
    c = h / cyls[drive];
    h %= heads[drive];

retry:
    *sm9234_wc = 0x40;			/* DMA pointer */
    *sm9234_wd = 0x00;			/* DMA low */
    *sm9234_wd = 0x00;			/* DMA med (for now - will be page) */
    *sm9234_wd = 0x00;			/* DMA high */
    *sm9234_wd = s;
    *sm9234_wd = h | (cyl & 0x700) >> 4);
    *sm9234_wd = c;
    *sm9234_wd = 2;			/* Always pairs, assume SPT is even */
    if (blk_op.is_read)
        *sm9234_wd = 0x00;
    else
        *sm9234_wd = 0xF0;
    *sm9234_wd = 0xE0;	/* FIXME step rate to pick ? */
    /* Not using IRQ yet */
    *sm9234_wd = 0x97;	/* Stop on ddm, wp, ready, write fault */
    *sm9234_wd = 0x10  | ((c >> 8) & 3);
    if (blk_op.is_read)
        *sm9234_wc = 0x5D;
    else {
        *sm9234_wd = 0xA0;	/* TODO: write precomp */
        hfdc_busy = 1;
        return 1;
    }
    /* Read wait for complete */
    while(!(*sm9234_rs & 0x20));
    /* Error check */
    /* TODO: reset/retry twice and also look at ECC scrubbing */
    /* TODO O_SYNC disk I/O write error paths */
    if (sm9234_rs & 0x0C) {
        hfdc_error();
        if (retry++ < 3) {
            hfdc_recover();
            goto retry;
        }
        return 0;
    }
    /* Copy any data out to the user */
    hfdc_cache_to_data();
    return 1;
}

int hfdc_flush_cache(void)
{
    hfdc_sync(drive);	/* For now */
    return 0;
}
