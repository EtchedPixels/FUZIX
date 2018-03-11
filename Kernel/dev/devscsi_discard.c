/*
 * Limits of our SCSI code
 * - No multi-lun
 * - No disconnect
 * - Simple 512 byte/sector disks only (easily fixed for 512, more is hard)
 * - No removable media (not too hard to fix - but removable disk is rare)
 * - No useful scsi ioctl API in scsi generic style (needs fixing)
 * - No CDROM (hard on 8bit)
 * - No tape etc
 */

#define _SCSI_PRIVATE

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <timer.h>
#include <devscsi.h>
#include <blkdev.h>

#ifdef CONFIG_SCSI

static uint8_t nscsi;
static uint8_t identify[36];
static uint8_t cap[8];

void scsi_dev_init(uint8_t drive)
{
  blkdev_t *blk;
  uint8_t *p;
  uint16_t secsize;

  /* FIXME: need to sort controller mapping policy here too */
  si_dcb.device = drive;
  si_dcb.lun = 0;
  si_dcb.bus = 0;
  if (si_select())		/* Can't select it - probably not present */
    return;
  /* FIXME: check if this would be better as a memcpy of fixed struct */
  si_dcb.cb.cb0.opcode = SIINQUIRY;
  si_dcb.cb.cb0.hiblock = 0;
  si_dcb.cb.cb0.miblock = 0;
  si_dcb.cb.cb0.loblock = 0;
  si_dcb.cb.cb0.noblocks = 36;
  si_dcb.cb.cb0.control = 0;  
  si_dcb.length = 36;
  si_dcb.direction = SIDIR_READ;
  si_user = 0;
  
  si_deselect();	/* As selects don't necessarily stack */
  
  if (si_docmd(identify))
    return;

  p = identify + 8;
  while (p < identify + 27)
    kputchar(*p++);
  kputchar('\n');

  /* Ok the device exists, but we may not be able to drive it */
  switch(identify[0] & 0x1F) {
  case 0x00:	/* Hard Disk */
  case 0x07:	/* Optical */
  case 0x0C:	/* RAID */
  case 0x0E:	/* RBC */
    break;
  default:
    return;
  }
  /* We should spin the device up I guess. We don't currently
     support removable media - that would need us to defer much of this to
     open and add an open hook to the blkdev layer */
     
  /* FIXME: use Test Unit ready - but note that for SASI at least TUR
     is optional */
  
  /* Read capacity tells us the disk size */
  memset(&si_dcb.cb.cb0, 0, sizeof(si_dcb.cb.cb0));
  si_dcb.cb.cb0.opcode = SIREAD_CAP;
  si_dcb.length = 8;
  si_dcb.direction = SIDIR_READ;
  si_user = 0;
  if (si_docmd(cap))
    return;
  
  if (cap[4] || cap[5])	/* Block size over 64K */
    return;
  secsize = (cap[6] << 8) | cap[7];
  if (secsize != 512) {
    kprintf("scsi: unsupported sector size %d\n", secsize);
    return;
  }
  
  /* Ok we pass. Allocate a disk device */
  blk = blkdev_alloc();
  if (!blk)
    return;
  blk->transfer = si_cmd;
  blk->flush = si_flush;
  blk->driver_data = drive;
  /* Very big disks report FFFFFFFF if they overrun this. We don't care we
     currently only speak READ6 anyway ! */
  blk->drive_lba_count = ((uint32_t)cap[0] << 24) | ((uint32_t)cap[1] << 16) | ((uint16_t)cap[2] << 8) | cap[3];
  blkdev_scan(blk, SWAPSCAN);
}

void devscsi_init(void)
{
  uint8_t i;
  /* This is a bit crude - we need to do proper scans of each controller
     according to devs/luns etc */
  for (i = 0; i < NSCSI; i++) {
    kprintf("\rSCSI drive %d: ", i);
    scsi_dev_init(i);
  }
  kprintf("\r%d SCSI device(s) detected.\n", nscsi);
}

#endif /* CONFIG_SCSI */
