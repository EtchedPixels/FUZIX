/*
 *	Minimalist SASI/SCSI interface
 *
 *	Does not support
 *	- Multi lun devices
 *	- Anything except disks (arguably we should have some kind of
 *	  'bus' interfacr to attach ioctls to for tapes etc
 *	- Any kind of multi-master scenario
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <tinydisk.h>
#include <tinyscsi.h>

#ifdef CONFIG_TD_SCSI

/* All this is discarded after boot on a small machine so we can be
   a bit more helpful */
static const uint8_t cmd_inquiry[6] = { 0x1F, 0x00, 0x00, 0x00, 36, 0x00};
static const uint8_t cmd_tur[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/* Trim the trailing spaces and print out the various identity strings
   if they are present */

static void pvid(uint8_t *p, uint_fast8_t len)
{
    uint8_t *e = p + len;
    while(e > p && *--e == ' ');
    while(p < e)
        kputchar(*p++);
    kputchar(' ');
}

static void scsi_devinfo(uint8_t *p)
{
    pvid(p + 8, 8);
    pvid(p + 16, 8);
    pvid(p + 4, 4);
}

static uint8_t scsi_probe_unit(uint_fast8_t n)
{
    uint8_t buf[36];
    uint8_t spintry = 0;
    kprintf("\r%d", n);

    /* Q: do earliest SASI devices have identify - eg the WREN ? */

    /* Do this command to kernel buffer */
    td_raw = 0;
    if (scsi_cmd(n, cmd_inquiry, buf, 36))
        return 0;

    /* We got a reply. So something exists */        
    if (scsi_status[0] & 0x02) {
        if (scsi_sense(n, buf) || scsi_status[0] & 0x02)
            return 0;
        /* Must be an early SASI drive ? */
        kputs(": SASI : ");
    } else {
        /* Only attach present DAD devices */
        if (buf[0] != 0x00) {
            kputs(": non-disk device.\n");
            return 0;
        }
        if (buf[4] < 32)
            kputs(": SCSI : ");
        else
            scsi_devinfo(buf);
    }
    /* Spinny spin spin */
    while(1) {
        if (scsi_cmd(n, cmd_tur, NULL, 0))
            break;
        /* Spun up */
        if (!(scsi_status[0] & 0x02))
            return 1;
        if (scsi_sense(n, buf))
            break;
        /* Unknown command: drive doesn't support TUR polling */
        if (buf[0] == 0x20)
            return 1;
        spintry++;
        kputchar("-\\|/"[spintry & 3]);
        kputchar(8);
    }
    kprintf("unable to spin up.\n");
    return 0;

}

/* Probe all the devices except for my own passed in as id */
void scsi_probe(uint_fast8_t my_id)
{
    uint8_t n = 0;
    int r;

    scsi_id = my_id;
    scsi_reset();

    /* We only support one controller with max 8 luns for now. We can treat
       the unit id in other ways later to handle more */
    for (n = 0; n < 8; n++) {
        if (n != my_id) {
            if (scsi_probe_unit(n)) {
                /* Attach the disk we found */
                r = td_register(n, scsi_xfer, td_ioctl_none, 1);
                if (r < 0)
                    continue;
            }
        }
    }
    kputchar('\r');
}

#endif
