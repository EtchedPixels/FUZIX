/*
 *	Generic core for the uPD765 floppy disk controllers
 *	Extracted from the Amstrad NC200 driver by David Given
 *
 *	The core work is in the platform specific asm file.
 *
 *
 *	TODO:
 *	Device types so we know about 40/80 and sides (and not present)	
 *	Asm C command issue hook for probing
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devfdc765.h>
#include <timer.h>
#include <platform_fdc765.h>

#ifdef CONFIG_FDC765

static timer_t spindown_timer, recal_timer;

static uint8_t lastdrive;
static uint8_t trackpos[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

int devfd_open(uint_fast8_t minor, uint16_t flag)
{
    flag;
    if(minor >= FDC765_MAX_FLOPPY || !(fdc765_present & (1 << minor)) ) {
        udata.u_error = ENODEV;
        return -1;
    }

    fd765_do_nudge_tc();

    trackpos[minor] = 0xFF;
    if (minor == lastdrive)
        fd765_track = 0xff; /* not on a known track */
    return 0;
}

static void nudge_timer(void)
{
    irqflags_t irq = di();
    spindown_timer = set_timer_sec(FDC_MOTOR_TIMEOUT);
    irqrestore(irq);
}

/* (called from interrupt context) */
void devfd_spindown(void)
{
    if (spindown_timer && timer_expired(spindown_timer))
    {
        fd765_motor_off();
        spindown_timer = 0;
    }
}

/* Seek to track 0. */
static uint_fast8_t fd_recalibrate(void)
{
    /*
     *	Keep trying to recalibrate until the command succeed or a
     *  motor timeout has passed.
     */
    recal_timer = set_timer_sec(FDC_MOTOR_TIMEOUT);

    do {
        nudge_timer();
        fd765_do_recalibrate();
        if (((fd765_status[0] & 0xf8) == 0x20) && !fd765_status[1]) {
            /* Forget which track we've saught to */
            /* Should we not just set it to 0 ??? FIXME */
            fd765_track = 0xFF;
            return 0;
        }
    } while(!timer_expired(recal_timer));
    /* Lost all hope */
    fd765_track = 0xff;
    return 1;
}

/* Set up the controller for a given block, seek, and wait for it.
   By the time we are called the motor is assumed to be at speed */

static uint_fast8_t fd_seek(uint_fast8_t minor, uint16_t lba)
{
    uint_fast8_t i = 0;
    uint_fast8_t track2, newtrack;

    if (fdc765_ds & (1 << minor)) {
        track2 = lba / 9;
        newtrack = track2 >> 1;

        fd765_sector = (lba % 9) + 1;
        fd765_head = track2 & 1;
    } else {
        newtrack = lba / 9;
        fd765_sector = (lba % 9) + 1;
        fd765_head = 0;
    }

    if (newtrack != fd765_track)
    {
        while (i++ < 5) {
            fd765_track = newtrack;
            nudge_timer();
            fd765_do_seek();
            if ((fd765_status[0] & 0xf8) == 0x20)
                return 0;
            if (i != 5)
                fd_recalibrate();
        }
        return 1;
    }
    return 0;
}

/* Select a drive and ensure the motor is on. */
static void fd_select(int minor)
{
    if (lastdrive != minor) {
        trackpos[lastdrive] = fd765_track;
        fd765_track = trackpos[minor];
        lastdrive = minor;
    }
    fd765_drive = minor;
    fd765_motor_on();
    nudge_timer();
}

static int devfd_transfer(uint_fast8_t minor, bool is_read, uint_fast8_t is_raw)
{
    int ct = 0;
    int tries;
    int blocks;
    uint16_t lba;

    /* This must happen first, as it updates the udata variables. */
    if (is_raw && d_blkoff(BLKSHIFT))
        return -1;
    blocks = udata.u_nblock;
    lba = udata.u_block;

    /* kprintf("[%s %d @ %x : %d:%x]\n", is_read ? "read" : "write",
          blocks, lba, is_raw, udata.u_dptr);
       if (!is_read)
          return blocks << BLKSHIFT;
    */

    fd_select(minor);
    fd765_is_user = is_raw;
    fd765_buffer = udata.u_dptr;

    while (blocks != 0)
    {
        for (tries = 0; tries < 3; tries ++)
        {
            nudge_timer();
            if (tries != 0) {
                /* Try recalibrating, but if that fails don't bother
                   trying the rest */
                if (fd_recalibrate())
                    continue;
            }
            /* Seek failed - no point trying the I/O again */
            if (fd_seek(minor, lba))
                continue;

            /* Not all machines can make the timing for this, or have
               real controllers that can do it */
#ifdef CONFIG_FDC765_MULTISECTOR
            fd765_sectors = 10 - fd765_sector;
            if (fd765_sectors > blocks)
                fd765_sectors = blocks;
#else
            fd765_sectors = 1;
#endif
            if (is_read)
                fd765_do_read();
            else
                fd765_do_write();

            /* Did it work ? */
            if ((fd765_status[0] & 0xc0) == 0)
                break;
        }
        if (tries == 3)
        {
            /* FIXME: will be the drive num once we fix that */
            kprintf("fd%d: I/O error %d:%d - %d:%d\n", minor , is_read, lba,
                            fd765_status[0], fd765_status[1]);
            udata.u_error = EIO;
            break;
        }
        lba += fd765_sectors;
        blocks -= fd765_sectors;
        ct += fd765_sectors;
    }

    return ct << BLKSHIFT;
}

int devfd_read(uint_fast8_t minor, uint_fast8_t is_raw, uint_fast8_t flag)
{
    flag;minor;
    return devfd_transfer(minor, true, is_raw);
}

int devfd_write(uint_fast8_t minor, uint_fast8_t is_raw, uint_fast8_t flag)
{
    flag;minor;
    return devfd_transfer(minor, false, is_raw);
}

#endif
