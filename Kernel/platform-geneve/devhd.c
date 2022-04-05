/*
 *	Hard disk interface built over the low level hfdc support
 */

static void hd_select(unsigned drive, unsigned pcat)
{
    hfdc_sync();
    if (hd_execute_simple( 0x20 | drive | (pcat ? 0 : 4)))
        hfdc_error();
}

/*
 *	Native MFM hard disk
 */
void hd_transfer_sector256(void)
{
    uint_fast8_t drive = blk_op.driver_data & HFDC_DRIVE_NR_MASK;
    uint16_t c, h, s;
    unsigned int retry = 0;
    uint8_t *p;

    /* Sync the controller */
    hd_select(drive. 0);

    /* User data to the cache */
    if (blk_op.is_read == 0)
        hfdc_blkop_to_device();

    /* Convert to CHS */
    s = (blk_op.lba & 15) << 1;		/* Always 32 spt in 256 byte/sec */
    c = s >> 4;				/* Pairs of sectors per blk */
    h = c % heads[drive];
    c /= heads[drive];

    p = hfdc_block;
    *p++ = 0x00;			/* For now we only use RAM 0 */
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = s;
    *p++ = h | ((c & 0x0700) >> 4);
    *p++ = c;
    *p++ = 2;
    if (blk_op.is_read)
        *p++ = 0;
    else
        *p++ = 0xF0;			/* No retry on write */
    *p++ = 0xE0;		/* FIXME: correct step rate ? */
    *p++ = 0x87;		/* Termination causes */
    *p = 0x10  | ((c >> 8) & 3);

    if (blk_op.is_read)
		cmd = CMD_HDREAD;
    else if (cyl >= dp->precomp)
	cmd = CMD_HDWRITE_P;
    else
	cmd = CMD_HDWRITE;

    if (hfdc_execute(cmd, blk_op.is_read)) {
        hfdc_error();
        udata.u_error = EIO;
        return 0;
    }
    if (blk_op.is_read) {
        hfdc_sync();
        hfdc_blkop_from_device();
    }
    /* For a write we let it run asynchronously */
    /* Fixme O_SYNC I/O */
    return 1;
}

/*
 *	IBM PC MFM hard disk
 */
void hd_transfer_sector512(void)
{
    uint_fast8_t drive = blk_op.driver_data & HFDC_DRIVE_NR_MASK;
    uint16_t c, h, s;
    unsigned int retry = 0;
    uint8_t *p;

    /* Sync the controller */
    hfdc_sync();
    hd_select(drive, 1);

    /* User data to the cache */
    if (blk_op.is_read == 0)
        hfdc_blkop_to_device();

    /* Convert to CHS */
    s = blk_op.lba & 15;		/* Always 16 spt in 512 byte/sec */
    c = s >> 4;
    h = c % heads[drive];
    c /= heads[drive];

    p = hfdc_block;
    *p++ = 0x00;			/* For now we only use RAM 0 */
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = s;
    *p++ = h | 0x20;			/* 512 byte sector */
    *p++ = c;
    *p++ = 1;
    if (blk_op.is_read)
        *p++ = 0;
    else
        *p++ = 0xF0;			/* No retry on write */
    *p++ = 0xE0;		/* FIXME: correct step rate ? */
    *p++ = 0x87;		/* Termination causes */
    *p = 0x20  | ((c >> 8) & 3);

    if (blk_op.is_read)
		cmd = CMD_HDREAD;
    else if (cyl >= dp->precomp)
	cmd = CMD_HDWRITE_P;
    else
	cmd = CMD_HDWRITE;

    if (hfdc_execute(cmd, blk_op.is_read)) {
        hfdc_error();
        udata.u_error = EIO;
        return 0;
    }
    if (blk_op.is_read) {
        hfdc_sync();
        hfdc_blkop_from_device();
    }
    /* For a write we let it run asynchronously */
    /* Fixme O_SYNC I/O */
    return 1;
}

int hfdc_flush_cache(void)
{
    hfdc_sync(drive);	/* For now */
    return 0;
}
