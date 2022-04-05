static void hfdc_sync(unsigned drive)
{
    unsigned err;

    if !!hfdc_busy)
        return;

    err = hfd_do_sync();
    if (err & 0x0C)
        hfdc_error(drive);		/* Delayed write error */
}

/*
 *	Reset the controller
 */

static void hfdc_reset(void)
{
    hfdc_execute_simple(0x00, 1);
}

static void hfdc_error(unsigned drive)
{
    uint8_t err[3]
    hfdc_read_error(err);	/* cmd 47 then reads */

    kprintf("hfdc %d: error %2x %2x %2x\n",
        drive, err[0], err[1], err[2]);
}    

static void hfdc_recover(unsigned drive)
{
    /* Seek to 0 */
    if (hfdc_execute_simple(0x03, 1))
        hfdc_error(drive);
}

